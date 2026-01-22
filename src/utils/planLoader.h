#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <random>

#include "nlohmann/json.hpp"

#include "core/data.h"
#include "core/utils.h"
#include "core/mesh.h"

using js = nlohmann::json;

namespace lmv
{
    inline lmcore::ERoomType cast_room_type(const std::string & type)
    {
        if(type == "Bedroom")
            return lmcore::ERoomType::Bedroom;
        if(type == "LivingRoom")
            return lmcore::ERoomType::LivingRoom;
        if(type == "Bathroom")
            return lmcore::ERoomType::Bathroom;
        if(type == "DiningRoom")
            return lmcore::ERoomType::DiningRoom;
        if(type == "Kitchen")
            return lmcore::ERoomType::Kitchen;
        return lmcore::ERoomType::ENUM_MAX;
    }

    inline lmcore::EOpeningType cast_opening_type(const std::string & type)
    {
        if(type == "door")
            return lmcore::EOpeningType::Door;
        if(type == "window")
            return lmcore::EOpeningType::Window;
        return lmcore::EOpeningType::ENUM_MAX;
    }

    class FloorPlanWallMerger
    {
        public:
            FloorPlanWallMerger(lmcore::FloorPlan plan) : mPlan(plan){}
            lmcore::FloorPlan Merge()
            {
                uint32_t room_count = mPlan.rooms.size();
                mRoomSegmentsCache.resize(room_count);
                
                getSegments(0,mRoomSegmentsCache[0]);
                uint32_t counter = 1;

                for(auto i = 1; i < room_count; i++)
                {
                    for(auto j = 0; j < counter; j++)
                    {
                        auto integrated = mRoomSegmentsCache[j];
                        getSegments(i,mRoomSegmentsCache[i]);
                        mergeSegments(mRoomSegmentsCache[j],mRoomSegmentsCache[i]);
                    }
                    counter++;
                }

                solidifyCache();
                mergeOpenings();
                solidifyWalls();

                return mPlan;
            }
        private:
            void solidifyWalls()
            {
                for(auto & wall : mPlan.walls)
                {
                    wall.data.faces.resize(2);
                    std::vector<lmcore::Vec3f> wall_points;
                    auto segwall = wall.value;
                    wall_points.push_back(segwall.start.value);
                    wall_points.push_back(segwall.end.value);

                    for(auto oi: wall.opening_indices)
                    {
                        auto opening = mPlan.openings[oi];
                        wall_points.push_back(opening.segment.start.value);
                        wall_points.push_back(opening.segment.end.value);
                    }

                    if(abs(segwall.start.value.x() - segwall.end.value.x()) >0.000001f)
                    {
                        std::sort(wall_points.begin(), wall_points.end(), [&](const lmcore::Vec3f & a, const lmcore::Vec3f & b) {
                            return a.x() < b.x();
                        });      
                    }
                    else
                    {
                        std::sort(wall_points.begin(), wall_points.end(), [&](const lmcore::Vec3f & a, const lmcore::Vec3f & b) {
                            return a.y() < b.y();
                        }); 
                    }

                    auto _equal = [](lmcore::Vec3f v1, lmcore::Vec3f v2)->bool
                    {
                        return (v1-v2).squaredNorm()<0.000001f;
                    };

                    std::vector<lmcore::Vec3f> wall_points_temp;
                    for(auto p : wall_points)
                    {
                        if(wall_points_temp.size() > 0 && _equal(wall_points_temp.back(),p))
                        {
                            continue;
                        }
                        wall_points_temp.push_back(p);
                    }

                    auto posadd = [](lmcore::PosColorVertex a, lmcore::Vec3f v) -> lmcore::PosColorVertex
                    {
                        a.x += v.x();
                        a.y += v.y();
                        a.z += v.z();
                        return a;
                    };

                    auto pushsquare = [](std::vector<lmcore::PosColorVertex> & vec, lmcore::PosColorVertex p0, lmcore::PosColorVertex p1, lmcore::PosColorVertex p2, lmcore::PosColorVertex p3)
                    {
                        vec.push_back(p0);
                        vec.push_back(p1);
                        vec.push_back(p2);
                        vec.push_back(p1);
                        vec.push_back(p3);
                        vec.push_back(p2);
                    };

                    auto pushband = [pushsquare](
                        std::vector<lmcore::PosColorVertex> & vec, 
                        lmcore::PosColorVertex p0, lmcore::PosColorVertex p1, lmcore::PosColorVertex p2, lmcore::PosColorVertex p3,
                        lmcore::PosColorVertex p0_, lmcore::PosColorVertex p1_, lmcore::PosColorVertex p2_, lmcore::PosColorVertex p3_
                    )
                    {
                        pushsquare(vec,p0_,p0,p2_,p2);
                        pushsquare(vec,p1,p1_,p3,p3_);
                        
                        float r = 0.5f;
                        float g = 0.6f;
                        float b = .8f;

                        p0.r = r;
                        p0.g = g;
                        p0.b = b;
                        p0_.r = r;
                        p0_.g = g;
                        p0_.b = b;

                        p1.r = r;
                        p1.g = g;
                        p1.b = b;
                        p1_.r = r;
                        p1_.g = g;
                        p1_.b = b;

                        p2.r = r;
                        p2.g = g;
                        p2.b = b;
                        p2_.r = r;
                        p2_.g = g;
                        p2_.b = b;

                        p3.r = r;
                        p3.g = g;
                        p3.b = b;
                        p3_.r = r;
                        p3_.g = g;
                        p3_.b = b;
                        pushsquare(vec,p0,p0_,p1,p1_);
                        pushsquare(vec,p2_,p2,p3_,p3);
                    };

                    int size = wall_points.size();
                    for(int i = 0; i < size-1; i++)
                    {
                        float sOff = 0.f;
                        float eOff = 0.f;
                        if(i == 0)
                            sOff = mPlan.data.global_wall_thickness/2.f;
                        if(i == size-2)
                            eOff = -mPlan.data.global_wall_thickness/2.f;

                        auto p0 = wall_points[i];
                        auto p1 = wall_points[i+1];

                        lmcore::Vec3f oDir = p0 - p1;
                        oDir.normalize();
                        p0 = p0 + oDir *sOff;
                        p1 = p1 + oDir *eOff;

                        bool is_opening = false;
                        for(auto oi: wall.opening_indices)
                        {
                            const auto & op = mPlan.openings[oi];
                            const auto & o0 = op.segment.start.value;
                            const auto & o1 = op.segment.end.value;

                            if((_equal(p0, o0)&&_equal(p1, o1))||(_equal(p1, o0)&&_equal(p0, o1)))
                            {
                                is_opening = true;
                                lmcore::PosColorVertex v0;
                                lmcore::PosColorVertex v1;
                                lmcore::PosColorVertex v2;
                                lmcore::PosColorVertex v3;

                                lmcore::PosColorVertex v0_p;
                                lmcore::PosColorVertex v1_p;
                                lmcore::PosColorVertex v2_p;
                                lmcore::PosColorVertex v3_p;

                                lmcore::PosColorVertex v0_n;
                                lmcore::PosColorVertex v1_n;
                                lmcore::PosColorVertex v2_n;
                                lmcore::PosColorVertex v3_n;
                                
                                v0.x = p0.x();
                                v0.y = p0.y();
                                v0.z = p0.z() + op.high;
                                
                                v1.x = p1.x();
                                v1.y = p1.y();
                                v1.z = p1.z() + op.high;

                                v2.x = p0.x();
                                v2.y = p0.y();
                                v2.z = p0.z() + mPlan.data.global_wall_height;
                                
                                v3.x = p1.x();
                                v3.y = p1.y();
                                v3.z = p1.z() + mPlan.data.global_wall_height;

                                lmcore::Vec3f dir = -lmcore::Vec3f{v1.x-v0.x, v1.y-v0.y, v1.z-v0.z}.cross(
                                    lmcore::Vec3f{v2.x-v1.x, v2.y-v1.y, v2.z-v1.z}
                                );
                                
                                dir.normalize();
                                dir = dir * mPlan.data.global_wall_thickness/2.f;

                                v0_p = posadd(v0, dir);
                                v1_p = posadd(v1, dir);
                                v2_p = posadd(v2, dir);
                                v3_p = posadd(v3, dir);

                                v0_n = posadd(v0, -dir);
                                v1_n = posadd(v1, -dir);
                                v2_n = posadd(v2, -dir);
                                v3_n = posadd(v3, -dir);

                                pushsquare(wall.data.faces[0].shape,v0_p,v1_p,v2_p,v3_p);
                                wall.data.faces[0].normal = dir;
                                pushsquare(wall.data.faces[1].shape,v1_n,v0_n,v3_n,v2_n);
                                wall.data.faces[1].normal = -dir;

                                pushband(wall.data.bands,
                                    v0_p,v1_p,v2_p,v3_p,
                                    v0_n,v1_n,v2_n,v3_n);
                        
                                wall.data.baseForm.push_back(v0);
                                wall.data.baseForm.push_back(v1);
                                wall.data.baseForm.push_back(v2);

                                wall.data.baseForm.push_back(v1);
                                wall.data.baseForm.push_back(v3);
                                wall.data.baseForm.push_back(v2);

                                if(op.type == lmcore::EOpeningType::Window)                               
                                {
                                    lmcore::PosColorVertex v0;
                                    lmcore::PosColorVertex v1;
                                    lmcore::PosColorVertex v2;
                                    lmcore::PosColorVertex v3;

                                    lmcore::PosColorVertex v0_p;
                                    lmcore::PosColorVertex v1_p;
                                    lmcore::PosColorVertex v2_p;
                                    lmcore::PosColorVertex v3_p;

                                    lmcore::PosColorVertex v0_n;
                                    lmcore::PosColorVertex v1_n;
                                    lmcore::PosColorVertex v2_n;
                                    lmcore::PosColorVertex v3_n;          

                                    v0.x = p0.x();
                                    v0.y = p0.y();
                                    v0.z = p0.z();
                                    
                                    v1.x = p1.x();
                                    v1.y = p1.y();
                                    v1.z = p1.z();

                                    v2.x = p0.x();
                                    v2.y = p0.y();
                                    v2.z = p0.z() + op.low;
                                    
                                    v3.x = p1.x();
                                    v3.y = p1.y();
                                    v3.z = p1.z() + op.low;

                                    lmcore::Vec3f dir = -lmcore::Vec3f{v1.x-v0.x, v1.y-v0.y, v1.z-v0.z}.cross(
                                        lmcore::Vec3f{v2.x-v1.x, v2.y-v1.y, v2.z-v1.z}
                                    );
                                
                                    dir.normalize();
                                    dir = dir * mPlan.data.global_wall_thickness / 2.f;

                                    v0_p = posadd(v0, dir);
                                    v1_p = posadd(v1, dir);
                                    v2_p = posadd(v2, dir);
                                    v3_p = posadd(v3, dir);

                                    v0_n = posadd(v0, -dir);
                                    v1_n = posadd(v1, -dir);
                                    v2_n = posadd(v2, -dir);
                                    v3_n = posadd(v3, -dir);

                                    pushsquare(wall.data.faces[0].shape,v0_p,v1_p,v2_p,v3_p);
                                    wall.data.faces[0].normal = dir;
                                    pushsquare(wall.data.faces[1].shape,v1_n,v0_n,v3_n,v2_n);
                                    wall.data.faces[1].normal = -dir;
                                    pushband(wall.data.bands,
                                    v0_p,v1_p,v2_p,v3_p,
                                    v0_n,v1_n,v2_n,v3_n);
                            
                                    wall.data.baseForm.push_back(v0);
                                    wall.data.baseForm.push_back(v1);
                                    wall.data.baseForm.push_back(v2);
                                    
                                    wall.data.baseForm.push_back(v1);
                                    wall.data.baseForm.push_back(v3);
                                    wall.data.baseForm.push_back(v2);
                                }
                            }
                        }

                        if(is_opening)
                            continue;

                        lmcore::PosColorVertex v0;
                        lmcore::PosColorVertex v1;
                        lmcore::PosColorVertex v2;
                        lmcore::PosColorVertex v3;

                        lmcore::PosColorVertex v0_p;
                        lmcore::PosColorVertex v1_p;
                        lmcore::PosColorVertex v2_p;
                        lmcore::PosColorVertex v3_p;

                        lmcore::PosColorVertex v0_n;
                        lmcore::PosColorVertex v1_n;
                        lmcore::PosColorVertex v2_n;
                        lmcore::PosColorVertex v3_n;          

                        v0.x = p0.x();
                        v0.y = p0.y();
                        v0.z = p0.z();
                        
                        v1.x = p1.x();
                        v1.y = p1.y();
                        v1.z = p1.z();

                        v2.x = p0.x();
                        v2.y = p0.y();
                        v2.z = p0.z() + mPlan.data.global_wall_height;
                        
                        v3.x = p1.x();
                        v3.y = p1.y();
                        v3.z = p1.z() + mPlan.data.global_wall_height;

                        lmcore::Vec3f dir = -lmcore::Vec3f{v1.x-v0.x, v1.y-v0.y, v1.z-v0.z}.cross(
                            lmcore::Vec3f{v2.x-v1.x, v2.y-v1.y, v2.z-v1.z}
                        );
                        
                        dir.normalize();
                        dir = dir * mPlan.data.global_floor_thickness / 2.f;

                        v0_p = posadd(v0, dir);
                        v1_p = posadd(v1, dir);
                        v2_p = posadd(v2, dir);
                        v3_p = posadd(v3, dir);

                        v0_n = posadd(v0, -dir);
                        v1_n = posadd(v1, -dir);
                        v2_n = posadd(v2, -dir);
                        v3_n = posadd(v3, -dir);

                        pushsquare(wall.data.faces[0].shape,v0_p,v1_p,v2_p,v3_p);
                        wall.data.faces[0].normal = dir;
                        pushsquare(wall.data.faces[1].shape,v1_n,v0_n,v3_n,v2_n);
                        wall.data.faces[1].normal = -dir;
                        pushband(wall.data.bands,
                        v0_p,v1_p,v2_p,v3_p,
                        v0_n,v1_n,v2_n,v3_n);
                
                        wall.data.baseForm.push_back(v0);
                        wall.data.baseForm.push_back(v1);
                        wall.data.baseForm.push_back(v2);
                        
                        wall.data.baseForm.push_back(v1);
                        wall.data.baseForm.push_back(v3);
                        wall.data.baseForm.push_back(v2);
                    }
                }
            }

            void mergeOpenings()
            {
                auto osize = mPlan.openings.size();
                std::vector<bool> visited(osize,false);
                for(auto & w : mPlan.walls)
                {
                    for(auto i = 0; i < osize; i++)
                    {
                        auto & op = mPlan.openings[i];
                        auto start = op.segment.start.value;
                        auto end = op.segment.end.value;

                        auto pos = op.position.value;

                        bool s_on = lmcore::is_point_on_segment(start,w.value);
                        bool e_on = lmcore::is_point_on_segment(end, w.value);

                        //todo, handle it
                        assert(!(s_on^e_on));
                        if(!(s_on && e_on))
                            continue;

                        assert(visited[i]==false);
                        visited[i] = true;
                        w.opening_indices.push_back(i);
                    }
                }
            }

            void solidifyCache()
            {
                int size = mRoomSegmentsCache.size();
                for(int i = 0; i < size; i++)
                {
                    auto & room_cache = mRoomSegmentsCache[i];
                    auto & room = mPlan.rooms[i];

                    for(auto rseg : room_cache)
                    {
                        int solidified = mPlan.walls.size();
                        bool exist = false;
                        for(auto j = 0; j < solidified; j++)
                        {
                            if(mPlan.walls[j].value == rseg)
                            {
                                room.wallIndices.push_back(j);
                                exist = true;
                                break;
                            }
                        }
                        if(exist)
                            continue;
                        mPlan.walls.push_back(lmcore::FPWallSegment{.value = rseg});
                        room.wallIndices.push_back(solidified);
                    }
                }
            }

            void getSegments(uint32_t room_i,std::vector<lmcore::FPLineSegment> & segs)
            {
                auto points = mPlan.rooms[room_i].geometries[0].points;
                segs.clear();
                for(auto i = 0; i < points.size(); i++)
                {
                    auto j = i+1;
                    if(i==points.size()-1)
                    {
                        j = 0;
                    }
                    segs.push_back(lmcore::FPLineSegment{.start = points[i],.end = points[j]});
                }
            }

            void cutSegmentsWithSingleSegment(std::vector<lmcore::FPLineSegment> & input, lmcore::FPLineSegment cutter)
            {
                std::vector<lmcore::FPLineSegment> output;
                for(auto i : input)
                {
                    auto res = lmcore::find_segment_intersection_xy(i,cutter);
                    if(res.hasIntersection)
                    {
                        for(auto idx:res.firstIndices)
                        {
                            output.push_back(res.newSegments[idx]);
                        }     
                    }
                    else
                        output.push_back(i);  
                }
                input.swap(output);
            }

            void mergeSegments(std::vector<lmcore::FPLineSegment> & ls0, std::vector<lmcore::FPLineSegment> & ls1)
            {
                uint32_t size0 = ls0.size();
                uint32_t size1 = ls1.size();
                std::vector<lmcore::FPLineSegment> temp0(ls0);
                std::vector<lmcore::FPLineSegment> temp1(ls1);
                for(auto c1 : ls1)
                {
                    cutSegmentsWithSingleSegment(temp0,c1);
                }

                for(auto c0: ls0)
                {
                    cutSegmentsWithSingleSegment(temp1,c0);
                }

                ls0.swap(temp0);
                ls1.swap(temp1);
            }
        public:
            std::vector<std::vector<lmcore::FPLineSegment>> mRoomSegmentsCache;
            lmcore::FloorPlan mPlan;
    };

    inline lmcore::FloorPlan load_floor_plan_from_json(const std::string & path)
    {
        lmcore::FloorPlan plan;
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << path <<std::endl;
            return plan;
        }
        js jdata;
        file >> jdata;
        
        auto rooms = jdata["rooms"];

        float offsetx = 0.f;
        float offsety = 0.f;
        uint32_t count = 0u;
        for(auto r : rooms)
        {
            lmcore::FPRoom rm;
            std::string name = r["name"];
            std::string type = r["type"];
            auto geo = r["geometry"];
            auto t = cast_room_type(type);
            rm.name = name;
            rm.type = t;
            for(auto _g : geo)
            {
                for(auto _p : _g)
                {
                    offsetx += float(_p[0]);
                    offsety += float(_p[1]);
                    count++;
                }
            }
        }

        offsetx /= count;
        offsety /= count;

        for(auto r : rooms)
        {
            lmcore::FPRoom rm;
            std::string name = r["name"];
            std::string type = r["type"];
            auto geo = r["geometry"];
            auto t = cast_room_type(type);

            rm.name = name;
            rm.type = t;

            for(auto _g : geo)
            {
                lmcore::FPGeometry g;
                for(auto _p : _g)
                {
                    lmcore::FPPoint p;
                    p.value.x() = float(_p[0]) - offsetx;
                    p.value.y() = float(_p[1]) - offsety;
                    p.value.z() = 0.0f;
                    g.points.push_back(p);
                }
                rm.geometries.push_back(g);
            }
            plan.rooms.push_back(rm);
        }

        auto openings = jdata["openings"];

        auto find_room = [](const std::string & name, const lmcore::FloorPlan & fp)->uint32_t
        {
            auto r_count = fp.rooms.size();
            for(auto i = 0; i < r_count; i++)
            {
                if(fp.rooms[i].name == name)
                    return i;
            }
            return -1;
        };

        for(auto o : openings)
        {
            std::string name = o["name"];
            std::string type = o["type"];
            auto position = o["position"];
            auto crooms = o["connected_rooms"];
            
            lmcore::FPOpening opening;
            opening.name = name;
            opening.type = cast_opening_type(type);
            
            lmcore::FPConnection connection;
            if(crooms.size()==1)
            {
                auto l = find_room(crooms[0],plan);
                opening.connection.first = l;
                opening.connection.out = true;
            }
            else if(crooms.size()==2)
            {
                auto l = find_room(crooms[0],plan);
                auto r = find_room(crooms[1],plan);
                opening.connection.first = l;
                opening.connection.second = r;
                opening.connection.out = false;
            }
            else
            {
                //todo throw exception
            }
            
            float x1 = float(position[0]) - offsetx;
            float x2 = float(position[2]) - offsetx;
            float y1 = float(position[1]) - offsety;
            float y2 = float(position[3]) - offsety;
            float tempz_h = 2.2f;

            opening.position.value = lmcore::Vec3f{(x1+x2)/2.f,(y1+y2)/2.f,0.f};
            lmcore::BBox bbox;
            bbox.xyz = lmcore::Vec3f(abs(x2-x1)/2.f,abs(y2-y1)/2.f,abs(tempz_h)/2.f);
            opening.bounding = bbox;

            lmcore::FPLineSegment seg;
            if(opening.bounding.xyz.x()>opening.bounding.xyz.y())
            {
                seg.start.value = opening.position.value - lmcore::Vec3f{opening.bounding.xyz.x(),0.f,0.f};
                seg.end.value = opening.position.value + lmcore::Vec3f{opening.bounding.xyz.x(),0.f,0.f};
            }
            else
            {
                seg.start.value = opening.position.value - lmcore::Vec3f{0.f, opening.bounding.xyz.y(), 0.f};
                seg.end.value = opening.position.value + lmcore::Vec3f{0.f, opening.bounding.xyz.y(), 0.f};
            }

            opening.segment = seg;
            if(opening.type == lmcore::EOpeningType::Window)
            {
                opening.low = plan.data.global_window_bottom_height;
                opening.high = plan.data.global_window_top_height;
            }
            else
            {
                opening.low = 0.f;
                opening.high = plan.data.global_door_height;
            }

            plan.openings.push_back(opening);
        }
        
        //to merge geometries
        std::vector<std::vector<uint32_t>> room_wall_indices;

        return plan;
    }

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