/* resource.c -- generic resource handling
 *
 * Copyright (C) 2010--2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "coap_config.h"
#include "coap_net.h"
#include "coap_debug.h"
#include "resource.h"
#include "subscribe.h"
#include "encode.h"//TODO
#include "utlist.h"
#include "soap_mem.h"

#define COAP_MALLOC_TYPE(Type) \
  ((coap_##Type##_t *)coap_malloc(sizeof(coap_##Type##_t)))
#define COAP_FREE_TYPE(Type, Object) coap_free(Object)

#define min(a,b) ((a) < (b) ? (a) : (b))

/* Helper functions for conditional output of character sequences into
 * a given buffer. The first Offset characters are skipped.
 */

/**
 * Adds Char to Buf if Offset is zero. Otherwise, Char is not written
 * and Offset is decremented.
 */
#define PRINT_WITH_OFFSET(Buf,Offset,Char)		\
  if ((Offset) == 0) {					\
    (*(Buf)++) = (Char);				\
  } else {						\
    (Offset)--;						\
  }							\

/**
 * Adds Char to Buf if Offset is zero and Buf is less than Bufend.
 */
#define PRINT_COND_WITH_OFFSET(Buf,Bufend,Offset,Char,Result) {		\
    if ((Buf) < (Bufend)) {						\
      PRINT_WITH_OFFSET(Buf,Offset,Char);				\
    }									\
    (Result)++;								\
  }

/**
 * Copies at most Length characters of Str to Buf. The first Offset
 * characters are skipped. Output may be truncated to Bufend - Buf
 * characters.
 */
#define COPY_COND_WITH_OFFSET(Buf,Bufend,Offset,Str,Length,Result) {	\
    size_t i;								\
    for (i = 0; i < (Length); i++) {					\
      PRINT_COND_WITH_OFFSET((Buf), (Bufend), (Offset), (Str)[i], (Result)); \
    }									\
  }

int
match(const str *text, const str *pattern, int match_prefix, int match_substring)
{
    assert(text);
    assert(pattern);

    if (text->length < pattern->length) {
        return 0;
    }

    if (match_substring) {
        unsigned char *next_token = text->s;
        size_t remaining_length = text->length;
        while (remaining_length) {
            size_t token_length;
            unsigned char *token = next_token;
            next_token = memchr(token, ' ', remaining_length);

            if (next_token) {
                token_length = next_token - token;
                remaining_length -= (token_length + 1);
                next_token++;
            } else {
                token_length = remaining_length;
                remaining_length = 0;
            }

            if ((match_prefix || pattern->length == token_length) &&
                memcmp(token, pattern->s, pattern->length) == 0) {
                return 1;
            }
        }
        return 0;
    }

    return (match_prefix || pattern->length == text->length) &&
           memcmp(text->s, pattern->s, pattern->length) == 0;
}

/**
 * Prints the names of all known resources to @p buf. This function
 * sets @p buflen to the number of bytes actually written and returns
 * @c 1 on succes. On error, the value in @p buflen is undefined and
 * the return value will be @c 0.
 *
 * @param context The context with the resource map.
 * @param buf     The buffer to write the result.
 * @param buflen  Must be initialized to the maximum length of @p buf and will be
 *                set to the length of the well-known response on return.
 * @param offset  The offset in bytes where the output shall start and is
 *                shifted accordingly with the characters that have been
 *                processed. This parameter is used to support the block
 *                option.
 * @param query_filter A filter query according to <a href="http://tools.ietf.org/html/draft-ietf-core-link-format-11#section-4.1">Link Format</a>
 *
 * @return COAP_PRINT_STATUS_ERROR on error. Otherwise, the lower 28 bits are
 *         set to the number of bytes that have actually been written to
 *         @p buf. COAP_PRINT_STATUS_TRUNC is set when the output has been
 *         truncated.
 */
#if defined(__GNUC__) && defined(WITHOUT_QUERY_FILTER)
coap_print_status_t
print_wellknown(coap_context_t *context, unsigned char *buf, size_t *buflen,
                size_t offset,
                coap_opt_t *query_filter __attribute__((unused)))
{
#else /* not a GCC */
coap_print_status_t
print_wellknown(coap_context_t *context, unsigned char *buf, size_t *buflen,
                size_t offset, coap_opt_t *query_filter)
{
#endif /* GCC */
    coap_resource_t *r;
    unsigned char *p = buf;
    const unsigned char *bufend = buf + *buflen;
    size_t left, written = 0;
    coap_print_status_t result;
    const size_t old_offset = offset;
    int subsequent_resource = 0;
#ifndef COAP_RESOURCES_NOHASH
    coap_resource_t *tmp;
#endif
#ifndef WITHOUT_QUERY_FILTER
    str resource_param = { 0, NULL }, query_pattern = { 0, NULL };
    int flags = 0; /* MATCH_SUBSTRING, MATCH_PREFIX, MATCH_URI */
#define MATCH_URI       0x01
#define MATCH_PREFIX    0x02
#define MATCH_SUBSTRING 0x04
    static const str _rt_attributes[] = {
        {2, (unsigned char *)"rt"},
        {2, (unsigned char *)"if"},
        {3, (unsigned char *)"rel"},
        {0, NULL}
    };
#endif /* WITHOUT_QUERY_FILTER */

#ifndef WITHOUT_QUERY_FILTER
    /* split query filter, if any */
    if (query_filter) {
        resource_param.s = COAP_OPT_VALUE(query_filter);
        while (resource_param.length < COAP_OPT_LENGTH(query_filter)
               && resource_param.s[resource_param.length] != '=') {
            resource_param.length++;
        }

        if (resource_param.length < COAP_OPT_LENGTH(query_filter)) {
            const str *rt_attributes;
            if (resource_param.length == 4 &&
                memcmp(resource_param.s, "href", 4) == 0) {
                flags |= MATCH_URI;
            }

            for (rt_attributes = _rt_attributes; rt_attributes->s; rt_attributes++) {
                if (resource_param.length == rt_attributes->length &&
                    memcmp(resource_param.s, rt_attributes->s, rt_attributes->length) == 0) {
                    flags |= MATCH_SUBSTRING;
                    break;
                }
            }

            /* rest is query-pattern */
            query_pattern.s =
                COAP_OPT_VALUE(query_filter) + resource_param.length + 1;

            assert((resource_param.length + 1) <= COAP_OPT_LENGTH(query_filter));
            query_pattern.length =
                COAP_OPT_LENGTH(query_filter) - (resource_param.length + 1);

            if ((query_pattern.s[0] == '/') && ((flags & MATCH_URI) == MATCH_URI)) {
                query_pattern.s++;
                query_pattern.length--;
            }

            if (query_pattern.length &&
                query_pattern.s[query_pattern.length - 1] == '*') {
                query_pattern.length--;
                flags |= MATCH_PREFIX;
            }
        }
    }
#endif /* WITHOUT_QUERY_FILTER */

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    HASH_ITER(hh, context->resources, r, tmp) {
#endif

#ifndef WITHOUT_QUERY_FILTER
        if (resource_param.length) { /* there is a query filter */

            if (flags & MATCH_URI) {	/* match resource URI */
                if (!match(&r->uri, &query_pattern, (flags & MATCH_PREFIX) != 0, (flags & MATCH_SUBSTRING) != 0)) {
                    continue;
                }
            } else {			/* match attribute */
                coap_attr_t *attr;
                str unquoted_val;
                attr = coap_find_attr(r, resource_param.s, resource_param.length);
                if (!attr) {
                    continue;
                }
                if (attr->value.s[0] == '"') {          /* if attribute has a quoted value, remove double quotes */
                    unquoted_val.length = attr->value.length - 2;
                    unquoted_val.s = attr->value.s + 1;
                } else {
                    unquoted_val = attr->value;
                }
                if (!(match(&unquoted_val, &query_pattern,
                            (flags & MATCH_PREFIX) != 0,
                            (flags & MATCH_SUBSTRING) != 0))) {
                    continue;
                }
            }
        }
#endif /* WITHOUT_QUERY_FILTER */

        if (!subsequent_resource) {	/* this is the first resource  */
            subsequent_resource = 1;
        } else {
            PRINT_COND_WITH_OFFSET(p, bufend, offset, ',', written);
        }

        left = bufend - p; /* calculate available space */
        result = coap_print_link(r, p, &left, &offset);

        if (result & COAP_PRINT_STATUS_ERROR) {
            break;
        }

        /* coap_print_link() returns the number of characters that
         * where actually written to p. Now advance to its end. */
        p += COAP_PRINT_OUTPUT_LENGTH(result);
        written += left;
    }

    *buflen = written;
    result = p - buf;
    if (result + old_offset - offset < *buflen) {
        result |= COAP_PRINT_STATUS_TRUNC;
    }
    return result;
}

coap_resource_t *
coap_resource_init(const unsigned char *uri, size_t len, int flags)
{
    coap_resource_t *r;

    r = (coap_resource_t *)coap_malloc(sizeof(coap_resource_t));
    if (r) {
        memset(r, 0, sizeof(coap_resource_t));

        LIST_STRUCT_INIT(r, subscribers);

        r->uri.s = (unsigned char *)uri;
        r->uri.length = len;

        coap_hash_path(r->uri.s, r->uri.length, r->key);

        r->flags = flags;
    } else {
        debug("coap_resource_init: no memory left\n");
    }

    return r;
}

coap_attr_t *
coap_add_attr(coap_resource_t *resource,
              const unsigned char *name, size_t nlen,
              const unsigned char *val, size_t vlen,
              int flags)
{
    coap_attr_t *attr;

    if (!resource || !name) {
        return NULL;
    }

    attr = (coap_attr_t *)coap_malloc(sizeof(coap_attr_t));

    if (attr) {
        attr->name.length = nlen;
        attr->value.length = val ? vlen : 0;

        attr->name.s = (unsigned char *)name;
        attr->value.s = (unsigned char *)val;

        attr->flags = flags;

        /* add attribute to resource list */

        LL_PREPEND(resource->link_attr, attr);
    } else {
        debug("coap_add_attr: no memory left\n");
    }

    return attr;
}

coap_attr_t *
coap_find_attr(coap_resource_t *resource,
               const unsigned char *name, size_t nlen)
{
    coap_attr_t *attr;

    if (!resource || !name) {
        return NULL;
    }

    LL_FOREACH(resource->link_attr, attr) {
        if (attr->name.length == nlen &&
            memcmp(attr->name.s, name, nlen) == 0) {
            return attr;
        }
    }

    return NULL;
}

void
coap_delete_attr(coap_attr_t *attr)
{
    if (!attr) {
        return;
    }
    if (attr->flags & COAP_ATTR_FLAGS_RELEASE_NAME) {
        coap_free(attr->name.s);
    }
    if (attr->flags & COAP_ATTR_FLAGS_RELEASE_VALUE) {
        coap_free(attr->value.s);
    }
#ifdef POSIX
    coap_free(attr);
#endif
}

void
coap_hash_request_uri(const coap_pdu_t *request, coap_key_t key)
{
    coap_opt_iterator_t opt_iter;
    coap_opt_filter_t filter;
    coap_opt_t *option;

    memset(key, 0, sizeof(coap_key_t));

    coap_option_filter_clear(filter);
    coap_option_setb(filter, COAP_OPTION_URI_PATH);

    coap_option_iterator_init((coap_pdu_t *)request, &opt_iter, filter);
    while ((option = coap_option_next(&opt_iter))) {
        coap_hash(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option), key);
    }
}

void
coap_add_resource(coap_context_t *context, coap_resource_t *resource)
{

#ifdef COAP_RESOURCES_NOHASH
    LL_PREPEND(context->resources, resource);
#else
    HASH_ADD(hh, context->resources, key, sizeof(coap_key_t), resource);
#endif
}

int
coap_delete_resource(coap_context_t *context, coap_key_t key)
{
    coap_resource_t *resource;
    coap_attr_t *attr, *tmp;

    if (!context) {
        return 0;
    }

    resource = coap_get_resource_from_key(context, key);

    if (!resource) {
        return 0;
    }

#ifdef COAP_RESOURCES_NOHASH
    LL_DELETE(context->resources, resource);
#else
    HASH_DELETE(hh, context->resources, resource);
#endif

    /* delete registered attributes */
    LL_FOREACH_SAFE(resource->link_attr, attr, tmp) coap_delete_attr(attr);

    if (resource->flags & COAP_RESOURCE_FLAGS_RELEASE_URI) {
        coap_free(resource->uri.s);
    }

    coap_free(resource);

    return 1;
}

coap_resource_t *
coap_get_resource_from_key(coap_context_t *context, coap_key_t key)
{
    coap_resource_t *resource;
#ifdef COAP_RESOURCES_NOHASH
    resource = NULL;
    LL_FOREACH(context->resources, resource) {
        /* if you think you can outspart the compiler and speed things up by (eg by
         * casting to uint32* and comparing alues), increment this counter: 1 */
        if (memcmp(key, resource->key, sizeof(coap_key_t)) == 0) {
            return resource;
        }
    }
    return NULL;
#else
    HASH_FIND(hh, context->resources, key, sizeof(coap_key_t), resource);

    return resource;
#endif
}

coap_resource_t *
coap_get_resource_from_uri(coap_context_t *context, char *uri)
{
    coap_resource_t *r;

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    coap_resource_t *tmp;
    HASH_ITER(hh, context->resources, r, tmp) {
#endif
        if (!strcmp((const char *)(r->uri.s), uri)) {
            return r;
        }
    }
    assert(r);
    return NULL;
}

coap_print_status_t
coap_print_link(const coap_resource_t *resource,
                unsigned char *buf, size_t *len, size_t *offset)
{
    unsigned char *p = buf;
    const unsigned char *bufend = buf + *len;
    coap_attr_t *attr;
    coap_print_status_t result = 0;
    const size_t old_offset = *offset;

    *len = 0;
    PRINT_COND_WITH_OFFSET(p, bufend, *offset, '<', *len);
    PRINT_COND_WITH_OFFSET(p, bufend, *offset, '/', *len);

    COPY_COND_WITH_OFFSET(p, bufend, *offset,
                          resource->uri.s, resource->uri.length, *len);

    PRINT_COND_WITH_OFFSET(p, bufend, *offset, '>', *len);


    LL_FOREACH(resource->link_attr, attr) {
        PRINT_COND_WITH_OFFSET(p, bufend, *offset, ';', *len);

        COPY_COND_WITH_OFFSET(p, bufend, *offset,
                              attr->name.s, attr->name.length, *len);

        if (attr->value.s) {
            PRINT_COND_WITH_OFFSET(p, bufend, *offset, '=', *len);

            COPY_COND_WITH_OFFSET(p, bufend, *offset,
                                  attr->value.s, attr->value.length, *len);
        }

    }
    if (resource->observable) {
        COPY_COND_WITH_OFFSET(p, bufend, *offset, ";obs", 4, *len);
    }

    result = p - buf;
    if (result + old_offset - *offset < *len) {
        result |= COAP_PRINT_STATUS_TRUNC;
    }

    return result;
}

#ifndef WITHOUT_OBSERVE
coap_subscription_t *
coap_find_observer(coap_resource_t *resource, const coap_address_t *peer,
                   const str *token)
{
    coap_subscription_t *s;

    assert(resource);
    assert(peer);

    for (s = list_head(resource->subscribers); s; s = list_item_next(s)) {
        if (coap_address_equals(&s->subscriber, peer)
            && (!token || (token->length == s->token_length
                           && memcmp(token->s, s->token, token->length) == 0))) {
            return s;
        }
    }

    return NULL;
}

coap_subscription_t *
coap_add_observer(coap_resource_t *resource,
                  const coap_address_t *observer,
                  const str *token)
{
    coap_subscription_t *s;

    assert(observer);

    /* Check if there is already a subscription for this peer. */
    s = coap_find_observer(resource, observer, token);

    /* We are done if subscription was found. */
    if (s) {
        return s;
    }

    /* s points to a different subscription, so we have to create
     * another one. */
    s = COAP_MALLOC_TYPE(subscription);

    if (!s) {
        return NULL;
    }

    coap_subscription_init(s);
    memcpy(&s->subscriber, observer, sizeof(coap_address_t));

    if (token && token->length) {
        s->token_length = token->length;
        memcpy(s->token, token->s, min(s->token_length, 8));
    }

    /* add subscriber to resource */
    list_t_add(resource->subscribers, s);

    return s;
}

void
coap_touch_observer(coap_context_t *context, const coap_address_t *observer,
                    const str *token)
{
    coap_resource_t *r;
    coap_subscription_t *s;

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    coap_resource_t *tmp;
    HASH_ITER(hh, context->resources, r, tmp) {
#endif
        s = coap_find_observer(r, observer, token);
        if (s) {
            s->fail_cnt = 0;
        }
    }
}

int
coap_delete_observer(coap_resource_t *resource, const coap_address_t *observer,
                     const str *token)
{
    coap_subscription_t *s;

    s = coap_find_observer(resource, observer, token);

    if (s) {
        list_remove(resource->subscribers, s);

        COAP_FREE_TYPE(subscription, s);
    }

    return s != NULL;
}

void
coap_notify_observers(coap_context_t *context, coap_resource_t *r)
{
    coap_method_handler_t h;
    coap_subscription_t *obs;
    str token;
    coap_pdu_t *response;

    if (r->observable && (r->dirty || r->partiallydirty)) {
        r->partiallydirty = 0;

        /* retrieve GET handler, prepare response */
        h = r->handler[COAP_REQUEST_GET - 1];
        assert(h);		/* we do not allow subscriptions if no
			 * GET handler is defined */

        for (obs = list_head(r->subscribers); obs; obs = list_item_next(obs)) {
            if (r->dirty == 0 && obs->dirty == 0)
                /* running this resource due to partiallydirty, but this observation's notification was already enqueued */
            {
                continue;
            }

            coap_tid_t tid = COAP_INVALID_TID;
            obs->dirty = 0;
            /* initialize response */
            response = coap_pdu_init(COAP_MESSAGE_CON, 0, 0, COAP_MAX_PDU_SIZE);
            if (!response) {
                obs->dirty = 1;
                r->partiallydirty = 1;
                debug("coap_check_notify: pdu init failed, resource stays partially dirty\n");
                continue;
            }

            if (!coap_add_token(response, obs->token_length, obs->token)) {
                obs->dirty = 1;
                r->partiallydirty = 1;
                debug("coap_check_notify: cannot add token, resource stays partially dirty\n");
                coap_delete_pdu(response);
                continue;
            }

            token.length = obs->token_length;
            token.s = obs->token;

            response->hdr->id = coap_new_message_id(context);
            if (obs->non && obs->non_cnt < COAP_OBS_MAX_NON) {
                response->hdr->type = COAP_MESSAGE_NON;
            } else {
                response->hdr->type = COAP_MESSAGE_CON;
            }
            /* fill with observer-specific data */
            h(context, r, &obs->subscriber, NULL, &token, response);

            if (response->hdr->type == COAP_MESSAGE_CON) {
                tid = coap_send_confirmed(context, &obs->subscriber, response);
                obs->non_cnt = 0;
            } else {
                tid = coap_send(context, &obs->subscriber, response);
                obs->non_cnt++;
            }

            if (COAP_INVALID_TID == tid || response->hdr->type != COAP_MESSAGE_CON) {
                coap_delete_pdu(response);
            }
            if (COAP_INVALID_TID == tid) {
                debug("coap_check_notify: sending failed, resource stays partially dirty\n");
                obs->dirty = 1;
                r->partiallydirty = 1;
            }

        }

        /* Increment value for next Observe use. */
        context->observe++;
    }
    r->dirty = 0;
}

void
coap_check_notify(coap_context_t *context)
{
    coap_resource_t *r;

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    coap_resource_t *tmp;
    HASH_ITER(hh, context->resources, r, tmp) {
#endif
        coap_notify_observers(context, r);
    }
}

/**
 * Checks the failure counter for (peer, token) and removes peer from
 * the list of observers for the given resource when COAP_OBS_MAX_FAIL
 * is reached.
 *
 * @param context  The CoAP context to use
 * @param resource The resource to check for (peer, token)
 * @param peer     The observer's address
 * @param token    The token that has been used for subscription.
 */
static void
coap_remove_failed_observers(coap_context_t *context,
                             coap_resource_t *resource,
                             const coap_address_t *peer,
                             const str *token)
{
    coap_subscription_t *obs;

    for (obs = list_head(resource->subscribers); obs;
         obs = list_item_next(obs)) {
        if (coap_address_equals(peer, &obs->subscriber) &&
            token->length == obs->token_length &&
            memcmp(token->s, obs->token, token->length) == 0) {

            /* count failed notifies and remove when
             * COAP_MAX_FAILED_NOTIFY is reached */
            if (obs->fail_cnt < COAP_OBS_MAX_FAIL) {
                obs->fail_cnt++;
            } else {
                list_remove(resource->subscribers, obs);
                obs->fail_cnt = 0;

#ifndef NDEBUG
                if (LOG_DEBUG <= coap_get_log_level()) {
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 40
#endif
                    unsigned char addr[INET6_ADDRSTRLEN + 8];

                    if (coap_print_addr(&obs->subscriber, addr, INET6_ADDRSTRLEN + 8)) {
                        debug("## removed observer %s\n", addr);
                    }
                }
#endif
                coap_cancel_all_messages(context, &obs->subscriber,
                                         obs->token, obs->token_length);

                COAP_FREE_TYPE(subscription, obs);
            }
            break;			/* break loop if observer was found */  //BUG FIX
        }
    }
}

void
coap_handle_failed_notify(coap_context_t *context,
                          const coap_address_t *peer,
                          const str *token)
{
    coap_resource_t *r;

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    coap_resource_t *tmp;
    HASH_ITER(hh, context->resources, r, tmp) {
#endif
        coap_remove_failed_observers(context, r, peer, token);
    }
}


static void coap_remove_all_failed_observers(coap_context_t *context,
        coap_resource_t *resource,
        const coap_address_t *peer)
{
    coap_subscription_t *obs;

    for (obs = list_head(resource->subscribers); obs;
         obs = list_item_next(obs)) {
        if (obs->subscriber.addr.sin.sin_addr.s_addr == peer->addr.sin.sin_addr.s_addr)
//        if (coap_address_equals(peer, &obs->subscriber))
        {
            list_remove(resource->subscribers, obs);
            obs->fail_cnt = 0;

#ifndef NDEBUG
            if (LOG_DEBUG <= coap_get_log_level()) {
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 40
#endif
                unsigned char addr[INET6_ADDRSTRLEN + 8];

                if (coap_print_addr(&obs->subscriber, addr, INET6_ADDRSTRLEN + 8)) {
                    debug("** removed observer %s\n", addr);
                }
            }
#endif
            coap_cancel_all_messages(context, &obs->subscriber,
                                     obs->token, obs->token_length);

            COAP_FREE_TYPE(subscription, obs);
        }
//        break;			/* break loop if observer was found */
    }
}

void coap_handle_all_failed_notifys(coap_context_t *context,
                                    const coap_address_t *peer)
{
    coap_resource_t *r;

#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, r) {
#else
    coap_resource_t *tmp;
    HASH_ITER(hh, context->resources, r, tmp) {
#endif
        coap_remove_all_failed_observers(context, r, peer);
    }
}


void coap_add_option_observer(coap_context_t  *ctx, struct coap_resource_t *resource,
                              coap_address_t *peer, coap_pdu_t *request, str *token, coap_pdu_t *response)
{
    char buf[4];
    coap_opt_iterator_t opt_iter;
    coap_subscription_t *subscription;

    if (request != NULL && coap_check_option(request, COAP_OPTION_OBSERVE, &opt_iter)) { //如果 APP 请求观察
        subscription = coap_add_observer(resource, peer, token);
        if (subscription) {
            subscription->non = request->hdr->type == COAP_MESSAGE_NON;
            coap_add_option(response, COAP_OPTION_OBSERVE, 0, NULL);
        }
    }
    if (resource->dirty == 1) {
        coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes((unsigned char *)buf, ctx->observe), (const unsigned char *)buf);
    }
}

#endif /* WITHOUT_NOTIFY */
