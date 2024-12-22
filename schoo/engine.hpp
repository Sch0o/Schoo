//
// Created by schoo on 2024/10/13.
//
#pragma once

#include<iostream>
#include"schoo/function/render/camera.hpp"
namespace schoo{

    class SchooEngine{
    public:
        const uint32_t width = 1600;
        const uint32_t height = 1200;

        float deltaTime=0.0f;
        //pre frame
        float lastTime=0.0f;

        //pre compute frame
        float lastSec=0.0f;
        int frameNums=0;

        std::shared_ptr<Camera>camera;

        static void Init();

        static void Quit();

        void TickMain();

        void TickRender();

        void TickLogic();

        void InitRender();

        void InitCamera();

        static SchooEngine&GetInstance();

        ~SchooEngine();
    private:
        SchooEngine()=default;
        static std::unique_ptr<SchooEngine>instance_;
    };


}

