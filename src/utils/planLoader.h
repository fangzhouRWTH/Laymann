#pragma once
#include <string>
#include <cstdint>
#include <random>
#include "core/data.h"


namespace lmcore
{
    class FloorPlanWallMerger
    {
        public:
            FloorPlanWallMerger(lmcore::FloorPlan plan) : mPlan(plan){}
            lmcore::FloorPlan Merge();
        private:
            void solidifyFloors();

            void solidifyWalls();

            void mergeOpenings();

            void solidifyCache();

            void getSegments(uint32_t room_i,std::vector<lmcore::FPLineSegment> & segs);

            void cutSegmentsWithSingleSegment(std::vector<lmcore::FPLineSegment> & input, lmcore::FPLineSegment cutter);

            void mergeSegments(std::vector<lmcore::FPLineSegment> & ls0, std::vector<lmcore::FPLineSegment> & ls1);
        public:
            std::vector<std::vector<lmcore::FPLineSegment>> mRoomSegmentsCache;
            lmcore::FloorPlan mPlan;
    };

    lmcore::FloorPlan load_floor_plan_from_json(const std::string & path);

    //temp
    inline void create_vertices_of_room_geometry(lmcore::FloorPlan & plan, std::vector<lmcore::PosColorVertex> & vertices)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0,1.0);
        for(auto room : plan.rooms)
        {
            auto size = room.geometries[0].points.size();
            for(auto i = 0; i < size; i++)
            {
                auto j = i+1;
                if(i == size - 1)
                    j = 0;
                auto ip = room.geometries[0].points[i].value;
                auto jp = room.geometries[0].points[j].value;
                lmcore::PosColorVertex iv;
                lmcore::PosColorVertex jv;
                iv.x = ip.x();
                iv.y = ip.y();
                iv.z = ip.z();

                float r = dis(gen);
                float g = dis(gen);
                float b = dis(gen);
                float a = 1.f;

                iv.r = r;
                iv.g = g;
                iv.b = b;
                iv.a = a;

                jv.x = jp.x();
                jv.y = jp.y();
                jv.z = jp.z();
                jv.r = r;
                jv.g = g;
                jv.b = b;
                jv.a = a;

                vertices.push_back(iv);vertices.push_back(jv);
                //indices.push_back(i);indices.push_back(i+1);
            }
        }
    }

    inline void create_vertices_indices_from_merger(FloorPlanWallMerger & merger, std::vector<lmcore::PosColorVertex> & vertices)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0,1.0);

        for(auto roomc : merger.mRoomSegmentsCache)
        {
            auto size = roomc.size();
            for(auto s : roomc)
            {
                float r = dis(gen);
                float g = dis(gen);
                float b = dis(gen);
                float a = 1.f;

                auto ip = s.start.value;
                auto jp = s.end.value;
                lmcore::PosColorVertex iv;
                lmcore::PosColorVertex jv;
                iv.x = ip.x();
                iv.y = ip.y();
                iv.z = ip.z();  
                iv.r = r;
                iv.g = g;
                iv.b = b;
                iv.a = a;

                jv.x = jp.x();
                jv.y = jp.y();
                jv.z = jp.z();
                jv.r = r;
                jv.g = g;
                jv.b = b;
                jv.a = a;

                // float z_offset = dis(gen);
                // iv.z = z_offset;
                // jv.z = z_offset;

                vertices.push_back(iv);vertices.push_back(jv);
            }
        }
    }
}