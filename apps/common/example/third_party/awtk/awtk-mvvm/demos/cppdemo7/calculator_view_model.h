﻿
/*This file is generated by code generator*/

#include "calculator.h"
#include "mvvm/base/view_model.h"

#ifndef TK_CALCULATOR_VIEW_MODEL_H
#define TK_CALCULATOR_VIEW_MODEL_H

BEGIN_C_DECLS
/**
 * @class calculator_view_model_t
 *
 * view model of Calculator
 *
 */
typedef struct _calculator_view_model_t {
    view_model_t view_model;

    /*model object*/
    Calculator *aCalculator;
} calculator_view_model_t;

/**
 * @method calculator_view_model_create
 * 创建Calculator view model对象。
 *
 * @annotation ["constructor"]
 * @param {navigator_request_t*} req 请求参数。
 *
 * @return {view_model_t} 返回view_model_t对象。
 */
view_model_t *calculator_view_model_create(navigator_request_t *req);

/**
 * @method calculator_view_model_create_with
 * 创建Calculator view model对象。
 *
 * @annotation ["constructor"]
 * @param {Calculator*}  aCalculator Calculator对象。
 *
 * @return {view_model_t} 返回view_model_t对象。
 */
view_model_t *calculator_view_model_create_with(Calculator *aCalculator);

/**
 * @method calculator_view_model_attach
 * 关联到Calculator对象。
 *
 * @param {view_model_t*} view_model view_model对象。
 * @param {Calculator*} Calculator Calculator对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t calculator_view_model_attach(view_model_t *vm, Calculator *aCalculator);

END_C_DECLS

#endif /*TK_CALCULATOR_VIEW_MODEL_H*/
