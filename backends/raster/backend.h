/*
Copyright (C) 2017  Paul Kremer

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../backend_interface.h"
#include "vec4.h"

// A rasterizer based on
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation
class RasterBackend : public BackendInterface
{
public:
    explicit RasterBackend(size_t width, size_t height);
    ~RasterBackend();

    int render(Picture& pic, const Mesh& mesh, const Vec3& view_pos);

private:
    size_t m_width = 0;
    size_t m_height = 0;
//    size_t m_size        = 0;
    Vec3 m_modelColor      = { 0 / 255.f, 120 / 255.f, 255 / 255.f }; // 模型颜色，蓝色
//    Vec3 m_modelColor      = { 254 / 255.f, 242 / 255.f, 58 / 255.f }; // 模型颜色，金色1
//    Vec3 m_modelColor      = { 255 / 255.f, 215 / 255.f, 0 / 255.f }; // 模型颜色，金色2
//    Vec3 m_modelColor      = { 205 / 255.f, 127 / 255.f, 50 / 255.f }; // 模型颜色，金色3
//    Vec3 m_modelColor      = { 166 / 255.f, 124 / 255.f, 64 / 255.f }; // 模型颜色，金色4
//    Vec3 m_modelColor      = { 217 / 255.f, 217 / 255.f, 25 / 255.f }; // 模型颜色，金色5
    Vec3 m_ambientColor    = { 0.4f, 0.4f, 0.4f }; // 环境光
    Vec3 m_diffuseColor    = { 0.2f, 0.2f, 0.2f }; // 漫反射光
    Vec3 m_specColor       = { 0.7f, 0.7f, 0.7f }; // 镜面反射光
    Vec3 m_lightPos        = { 0.f, 2.f, 0.f }; // 光源位置 // Vec3 m_lightPos        = { 2.0f, 2.0f, 2.5f }; // 光源位置

//    unsigned m_size        = 0;
//    Vec3 m_modelColor      = { 0 / 255.f, 120 / 255.f, 255 / 255.f }; // 模型颜色，蓝色
//    Vec3 m_ambientColor    = { 0.0f, 0.0f, 0.4f }; // 环境光
//    Vec3 m_diffuseColor    = { 0.0f, 0.5f, 1.0f }; // 漫反射光
//    Vec3 m_specColor       = { 1.0f, 1.0f, 1.0f }; // 镜面反射光
//    Vec4 m_backgroundColor = { 1.0f, 1.0f, 1.0f, 0.0f }; // 背景色，1,1,1,0表示白色
//    Vec3 m_lightPos        = { 2.0f, 2.0f, 2.5f }; // 光源位置
};

//常用金色的rgb值
//R=255，G=215，B=0
//R=205，G=127，B=50
//R=166，G=124，B=64
//R=217，G=217，B=25
