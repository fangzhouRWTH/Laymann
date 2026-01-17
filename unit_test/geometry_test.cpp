#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "core/utils.h"
#include "core/data.h"

bool is_segment_matched(const std::vector<lmcore::FPLineSegment> & first,const std::vector<lmcore::FPLineSegment> & second)
{
    if(first.size()!=second.size())
        return false;
    int size = first.size();
    std::vector<bool> fv(size,false);
    std::vector<bool> sv(size,false);

    for(auto i = 0; i < size; i++)
    {
        for(auto j = 0; j < size; j++)
        {
            if(sv[j])
                continue;
            
            if(first[i]==second[j])
            {    
                fv[i] = true;
                sv[j] = true;
            }
        }
    }

    for(auto b : fv)
    {
        if(!b)
            return false;
    }

    for(auto b : sv)
    {
        if(!b)
            return false;
    }

    return true;
}

bool compare_segs(const lmcore::IntersectionResult & res, const std::vector<lmcore::FPLineSegment> & first, const std::vector<lmcore::FPLineSegment> & second)
{
    std::vector<lmcore::FPLineSegment> tempfs;
    std::vector<lmcore::FPLineSegment> tempss;

    for(auto f : res.firstIndices)
    {
        tempfs.push_back(res.newSegments[f]);
    }
    for(auto s : res.secondIndices)
    {
        tempss.push_back(res.newSegments[s]);
    }

    return is_segment_matched(tempfs,first)&&is_segment_matched(tempss,second);
}

std::vector<lmcore::FPLineSegment> create_se_list(std::vector<float> ls)
{
    if(ls.size()%6u!=0u)
        return {};
    
    std::vector<lmcore::FPLineSegment> res;
    for(int i = 0; i < ls.size(); i+=6)
    {
        res.push_back(
            lmcore::FPLineSegment{
                .start = lmcore::FPPoint{.value = lmcore::Vec3f{ls[i],ls[i+1],ls[i+2]}},
                .end = lmcore::FPPoint{.value = lmcore::Vec3f{ls[i+3],ls[i+4],ls[i+5]}}}
        );
    }
    return res;
}

lmcore::FPLineSegment create_seg(float fx, float fy, float fz, float sx, float sy, float sz)
{
    return lmcore::FPLineSegment{lmcore::Vec3f{fx,fy,fz},lmcore::Vec3f{sx,sy,sz}};
}

TEST_CASE("GEOMETRY") {
    bool bres = false;
    lmcore::FPLineSegment l1;
    lmcore::FPLineSegment l2;
    std::vector<lmcore::FPLineSegment> fcompare;
    std::vector<lmcore::FPLineSegment> scompare;

    l1 = create_seg(-10.0,1.0,1.0,10.0,1.0,1.0);
    l2 = create_seg(-2.0,1.0,1.0,2.0,1.0,1.0);
    auto res = lmcore::find_segment_intersection_xy(l1,l2);
    fcompare = create_se_list(
        {
            -10.0,1.0,1.0,
            -2.0,1.0,1.0,
            -2.0,1.0,1.0,
            2.0,1.0,1.0,
            2.0,1.0,1.0,
            10.0,1.0,1.0
        }
    );
    scompare = create_se_list(
        {
            -2.0,1.0,1.0,
            2.0,1.0,1.0
        }
    );
    bres = compare_segs(res,fcompare,scompare);
    REQUIRE(bres);
    
    l1 = create_seg(-10.f,3.f,0.f,-1.f,3.f,0.f);
    l2 = create_seg(-5.f,3.f,0.f,10.f,3.f,0.f);
    fcompare = create_se_list({-10.f,3.f,0.f,-5.f,3.f,0.f,-5.f,3.f,0.f,-1.f,3.f,0.f,});
    scompare = create_se_list({-5.f,3.f,0.f,-1.f,3.f,0.f,-1.f,3.f,0.f,10.f,3.f,0.f});
    res = lmcore::find_segment_intersection_xy(l1,l2);
    bres = compare_segs(res,fcompare,scompare);
    REQUIRE(bres);

    l1 = create_seg(1.f,0.f,0.f,2.f,0.f,0.f);
    l2 = create_seg(2.f,0.f,0.f,5.f,0.f,0.f);
    fcompare = create_se_list({1.f,0.f,0.f,2.f,0.f,0.f});
    scompare = create_se_list({2.f,0.f,0.f,5.f,0.f,0.f});
    res = lmcore::find_segment_intersection_xy(l1,l2);
    bres = compare_segs(res,fcompare,scompare);
    REQUIRE((bres&&res.hasIntersection));

    l1 = create_seg(1.f,0.f,0.f,2.f,0.f,0.f);
    l2 = create_seg(4.f,0.f,0.f,5.f,0.f,0.f);
    res = lmcore::find_segment_intersection_xy(l1,l2);
    REQUIRE(res.hasIntersection==false);
}

TEST_CASE("MESH OPERATION")
{

}