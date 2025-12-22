#ifndef RAYCASTER_H
#define RAYCASTER_H

#include<glm/glm.hpp>
#include "build.h"

class Raycaster
{
public:
    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 dir;
    };

    static Ray ScreenToWorldRay(double mouseX,double mouseY,int width,int height, const glm::mat4& view,const glm::mat4& projection)
    {
        float x=(2.0f*(float)mouseX/width-1.0f);
        float y=1.0f-(2.0f*(float)mouseY)/height;

        glm::mat4 invVP=glm::inverse(projection*view);

        glm::vec4 nearPoint=invVP*glm::vec4(x,y,-1.0f,1.0f);
        glm::vec4 farPoint=invVP*glm::vec4(x,y,1.0f,1.0f);
        nearPoint/=nearPoint.w;
        farPoint/=farPoint.w;

        Ray ray;
        ray.origin=glm::vec3(nearPoint);
        ray.dir=glm::normalize(glm::vec3(farPoint-nearPoint));
        return ray;
    }
    //求交点
    static bool RayAABB(const Ray& ray,
                    const glm::vec3& aabbMin,
                    const glm::vec3& aabbMax,
                    float& tOut)
    {
        float tmin = (aabbMin.x - ray.origin.x) / ray.dir.x;
        float tmax = (aabbMax.x - ray.origin.x) / ray.dir.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (aabbMin.y - ray.origin.y) / ray.dir.y;
        float tymax = (aabbMax.y - ray.origin.y) / ray.dir.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return false;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (aabbMin.z - ray.origin.z) / ray.dir.z;
        float tzmax = (aabbMax.z - ray.origin.z) / ray.dir.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return false;
        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        tOut = tmin;
        return tOut >= 0.0f;
    }


    static void PickObject(double mouseX, double mouseY,
                       int width, int height,
                       const glm::mat4& view,
                       const glm::mat4& projection,
                       BuildSystem& build)
    {
        Ray ray = ScreenToWorldRay(mouseX, mouseY, width, height, view, projection);

        float closestT = 1e9f;
        int hitId = -1;

        for (auto& obj : build.objects)
        {
            // 计算世界空间 AABB（用 8 个角点变换）
            glm::vec3 corners[8] = {
                {-0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f},{-0.5f, 0.5f,-0.5f},
                {-0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f},{-0.5f, 0.5f, 0.5f}
            };

            glm::vec3 minP( 1e9f), maxP(-1e9f);
            for (int i = 0; i < 8; ++i)
            {
                glm::vec4 w = obj.model * glm::vec4(corners[i], 1.0f);
                minP = glm::min(minP, glm::vec3(w));
                maxP = glm::max(maxP, glm::vec3(w));
            }

            float t;
            if (RayAABB(ray, minP, maxP, t))
            {
                if (t < closestT)
                {
                    closestT = t;
                    hitId = obj.id;
                }
            }
        }

        build.selectedId = hitId;
        for (auto& obj : build.objects)
            obj.selected = (obj.id == hitId);
    }

};
#endif
