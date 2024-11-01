﻿
/*This file is generated by code generator*/

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "mvvm/base/utils.h"
#include "humidity_view_model.h"

static ret_t humidity_view_model_set_prop(tk_object_t *obj, const char *name, const value_t *v)
{
    Humidity *aHumidity = ((humidity_view_model_t *)(obj))->aHumidity;

    if (tk_str_ieq("value", name)) {
        aHumidity->value = value_double(v);

        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t humidity_view_model_get_prop(tk_object_t *obj, const char *name, value_t *v)
{
    Humidity *aHumidity = ((humidity_view_model_t *)(obj))->aHumidity;

    if (tk_str_ieq("value", name)) {
        value_set_double(v, aHumidity->value);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static bool_t humidity_view_model_can_exec(tk_object_t *obj, const char *name, const char *args)
{
    humidity_view_model_t *vm = (humidity_view_model_t *)(obj);
    Humidity *aHumidity = vm->aHumidity;
    if (tk_str_ieq("Apply", name)) {
        return aHumidity->CanApply();
    }
    return FALSE;
}

static ret_t humidity_view_model_exec(tk_object_t *obj, const char *name, const char *args)
{
    humidity_view_model_t *vm = (humidity_view_model_t *)(obj);
    Humidity *aHumidity = vm->aHumidity;
    if (tk_str_ieq("Apply", name)) {
        return aHumidity->Apply();
    }
    return RET_NOT_FOUND;
}

static ret_t humidity_view_model_on_destroy(tk_object_t *obj)
{
    humidity_view_model_t *vm = (humidity_view_model_t *)(obj);
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);

    delete (vm->aHumidity);

    return view_model_deinit(VIEW_MODEL(obj));
}

static const object_vtable_t s_humidity_view_model_vtable = {"humidity_view_model_t",
                                                             "humidity_view_model_t",
                                                             sizeof(humidity_view_model_t),
                                                             FALSE,
                                                             humidity_view_model_on_destroy,
                                                             NULL,
                                                             humidity_view_model_get_prop,
                                                             humidity_view_model_set_prop,
                                                             NULL,
                                                             NULL,
                                                             humidity_view_model_can_exec,
                                                             humidity_view_model_exec
                                                            };

view_model_t *humidity_view_model_create_with(Humidity *aHumidity)
{
    tk_object_t *obj = tk_object_create(&s_humidity_view_model_vtable);
    view_model_t *vm = view_model_init(VIEW_MODEL(obj));
    humidity_view_model_t *humidity_view_model = (humidity_view_model_t *)(vm);

    return_value_if_fail(vm != NULL, NULL);

    humidity_view_model->aHumidity = aHumidity;

    return vm;
}

ret_t humidity_view_model_attach(view_model_t *vm, Humidity *aHumidity)
{
    humidity_view_model_t *humidity_view_model = (humidity_view_model_t *)(vm);
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);

    humidity_view_model->aHumidity = aHumidity;

    return RET_OK;
}

view_model_t *humidity_view_model_create(navigator_request_t *req)
{
    Humidity *aHumidity = new Humidity();
    return_value_if_fail(aHumidity != NULL, NULL);

    return humidity_view_model_create_with(aHumidity);
}
