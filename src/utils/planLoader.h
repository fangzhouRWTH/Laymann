#include <string>
#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"

#include "core/data.h"

using js = nlohmann::json;

namespace lmv
{
    lmcore::ERoomType cast_room_type(const std::string & type)
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

    lmcore::EOpeningType cast_opening_type(const std::string & type)
    {
        if(type == "Door")
            return lmcore::EOpeningType::Door;
        if(type == "Window")
            return lmcore::EOpeningType::Window;
        return lmcore::EOpeningType::ENUM_MAX;
    }

    lmcore::FloorPlan load_floor_plan_from_json(const std::string & path)
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
                    p.value.x() = _p[0];
                    p.value.y() = _p[1];
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
            
            float x1 = position[0];
            float x2 = position[2];
            float y1 = position[1];
            float y2 = position[3];
            float tempz_h = 2.2f;

            opening.position.value = lmcore::Vec3f{(x1+x2)/2.f,(y1+y2)/2.f,0.f};
            lmcore::BBox bbox;
            bbox.xyz = lmcore::Vec3f((x2-x1)/2.f,(y2-y1)/2.f,tempz_h/2.f);
            opening.bounding = bbox;

            plan.openings.push_back(opening);
        }
        return plan;
    }
}