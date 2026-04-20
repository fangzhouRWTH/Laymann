#pragma once
#include <vector>
#include <cstdint>
#include <queue>
#include <memory>
#include <cassert>

#include "citygen/type.h"
#include "citygen/random.h"
#include "citygen/utils.h"
#include "citygen/noise.h"
#include "citygen/field.h"

namespace lmcore
{
    template <typename T>
    struct SparseVector
    {
        std::vector<std::pair<bool, uint32_t>> indices;
        std::queue<uint32_t> idle_indices;
        std::vector<T> content;
        uint32_t size = 0u;

        uint32_t get_full_size()
        {
            return content.size();
        }

        uint32_t get_size()
        {
            return size;
        }

        uint32_t add(T t)
        {
            uint32_t idx;
            if (!idle_indices.empty())
            {
                idx = idle_indices.front();
                idle_indices.pop();
                indices[idx].first = true;
            }
            else
            {
                auto &item = indices.emplace_back();
                uint32_t cidx = content.size();
                auto &co = content.emplace_back(t);
                item.first = true;
                item.second = cidx;
            }

            size++;

            return idx;
        }

        void remove(uint32_t idx)
        {
            assert(is_valid(idx));
            indices[idx].first = false;
            idle_indices.push(idx);

            size--;
        }

        const bool is_valid(uint32_t idx) const
        {
            if (idx < indices.size() && indices[idx].first)
                return true;
            assert(false);
            return false;
        }

        T &get(uint32_t idx)
        {
            assert(is_valid(idx));
            return content[indices[idx].second];
        }

        const T &get(uint32_t idx) const
        {
            assert(is_valid(idx));
            return content[indices[idx].second];
        }

        T &operator[](uint32_t idx)
        {
            return get(idx);
        }

        const T &operator[](uint32_t idx) const
        {
            return get(idx);
        }

        std::vector<T> getArray()
        {
            std::vector<T> array;

            for (auto &i : indices)
            {
                if (i.first)
                    array.push_back(content[i.second]);
            }

            return std::move(array);
        }

        std::vector<uint32_t> getIndicesArray()
        {
            std::vector<uint32_t> array;

            for (auto &i : indices)
            {
                if (i.first)
                    array.push_back(i.second);
            }

            return std::move(array);
        }
    };

#define MAX_NODE_EDGE_COUNT 8u
    struct RoadNode
    {
        GVec3f pos;
        const static uint32_t sMaxEdgeCount = MAX_NODE_EDGE_COUNT;
        uint32_t edgeIndice[sMaxEdgeCount];
        uint32_t edgeCount = 0u;
    };

    struct RoadSegment
    {
        int startNode;
        int endNode;
        int level;
        bool active;
    };

    struct GrowthFront
    {
        int nodeId;
        GVec2f dir;
        int level;
        float length;
        int parentSegment;
        GVec2f target;
    };

    struct RoadNet
    {
        explicit RoadNet(float width, float height, uint32_t seed) : regionHalfWidth(width / 2.f), regionHalfHeight(height / 2.f), random(seed)
        {
        }

        float regionHalfWidth;
        float regionHalfHeight;

        // std::vector<RoadNode> nodes;
        // std::vector<RoadSegment> segments;

        SparseVector<RoadNode> nodes;
        SparseVector<RoadSegment> segments;

        std::queue<GrowthFront> fronts;
        lmcore::Random random;
    };

    class RoadNetBuilder
    {
    public:
        RoadNet get()
        {
            return mNet;
        }

        RoadNode &get_node(uint32_t idx)
        {
            assert(idx < mNet.nodes.get_size());
            return mNet.nodes[idx];
        }

        RoadSegment &get_edge(uint32_t idx)
        {
            assert(idx < mNet.segments.get_size());
            return mNet.segments[idx];
        }

        uint32_t add_node(GVec3f pos)
        {
            RoadNode node;
            node.pos = pos;

            uint32_t idx = mNet.nodes.get_size();
            mNet.nodes.add(node);

            return idx;
        }

        uint32_t add_edge(uint32_t idx1, uint32_t idx2, int level)
        {
            uint32_t nodeCount = mNet.nodes.get_size();
            assert(idx1 < nodeCount && idx2 < nodeCount);
            auto &n1 = mNet.nodes[idx1];
            auto &n2 = mNet.nodes[idx2];
            assert(check_edge_count(idx1) && check_edge_count(idx2));
            assert(!has_edge(idx1, idx2));
            uint32_t idx = mNet.segments.get_size();
            RoadSegment seg;
            seg.startNode = idx1;
            seg.endNode = idx2;
            seg.level = level;
            mNet.segments.add(seg);
            n1.edgeIndice[n1.edgeCount] = idx;
            n1.edgeCount++;
            n2.edgeIndice[n2.edgeCount] = idx;
            n2.edgeCount++;

            return idx;
        }

        void remove_node(uint32_t idx)
        {
            assert(check_node(idx));
            auto n = mNet.nodes[idx];
            uint32_t segcount = n.edgeCount;
            for(auto si = 0; si < segcount; si++)
            {
                auto seg = mNet.segments[si];
                auto n1 = seg.startNode;
                auto n2 = seg.endNode;

                uint32_t other = idx == n1? n2 : n1;
                auto & otherN = mNet.nodes[other];
                uint32_t eCount = 0;
                for(auto i = 0; i < otherN.edgeCount; i++)
                {
                    if(otherN.edgeIndice[i] != si)
                    {
                        otherN.edgeIndice[eCount++] = otherN.edgeIndice[i];
                    }
                }
                otherN.edgeCount = eCount;

                mNet.segments.remove(si);
            }
            
            mNet.nodes.remove(idx);
        }

        void remove_edge(uint32_t idx1, uint32_t idx2)
        {
            assert(check_node(idx1)&&check_node(idx2));
            auto n1 = mNet.nodes[idx1];
            auto n2 = mNet.nodes[idx2];

            
        }

    private:
        bool check_node(uint32_t idx) const
        {
            return mNet.nodes.is_valid(idx);
        }

        bool check_edge(uint32_t idx) const
        {
            return mNet.segments.is_valid(idx);
        }

        bool check_edge_count(uint32_t idx) const
        {
            return mNet.nodes[idx].edgeCount < mNet.nodes[idx].sMaxEdgeCount;
        }

        bool has_edge(uint32_t idx1, uint32_t idx2)
        {
            assert(check_node(idx1) && check_node(idx2));
            auto &n1 = mNet.nodes[idx1];
            auto &n2 = mNet.nodes[idx2];

            for (uint32_t eidx = 0; eidx < n1.edgeCount; eidx++)
            {
                assert(check_edge(eidx));
                auto &e = mNet.segments[eidx];
                if ((e.startNode == idx1 && e.endNode == idx2) || (e.startNode == idx2 && e.endNode == idx1))
                {
                    return true;
                }
            }
            return false;
        }

        RoadNet mNet;
    };

    class GrowthPolicy
    {
    public:
    private:
    };

    class L1Growth
    {
    public:
        L1Growth(std::shared_ptr<Field1F> field1) : mField1(field1)
        {
            assert(field1 != nullptr);
        }

        ~L1Growth()
        {
        }

        void Grow(RoadNet &net, uint32_t steps, float length, uint32_t origins)
        {

            init(net, origins);
            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = net.fronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = net.fronts.front();
                    net.fronts.pop();
                    RoadNode n = net.nodes[f.nodeId];
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, length);

                    if (pr.first.level < 0)
                        continue;

                    auto potentialfront = pr.first;
                    auto potentialnode = pr.second;

                    bool has_intersect = false;
                    int int_node_id = -1;
                    GVec3f newPos;
                    auto segidx = net.segments.getIndicesArray();
                    for (auto sidx : segidx)
                    {
                        auto &s = net.segments.get(sidx);
                        auto s0 = net.nodes[s.startNode].pos;
                        auto s1 = net.nodes[s.endNode].pos;
                        auto s2 = net.nodes[f.nodeId].pos;
                        auto s3 = pr.second.pos;

                        has_intersect = segmentsProperlyIntersect2D(s0, s1, s2, s3);
                        if (has_intersect)
                        {
                            int_node_id = s.startNode;
                            newPos.x = (s0.x + s1.x + s2.x + s3.x) / 4.f;
                            newPos.y = (s0.y + s1.y + s2.y + s3.y) / 4.f;
                            newPos.z = mField1->Sample(newPos.x, newPos.y);
                            break;
                        }
                    }

                    if (has_intersect)
                    {
                        pr.first.nodeId = int_node_id;
                        net.nodes[int_node_id].pos = newPos;
                    }
                    else
                    {
                        uint32_t nidx = net.nodes.get_size();
                        net.nodes.add(pr.second);
                        pr.first.nodeId = nidx;
                    }
                    net.fronts.push(pr.first);

                    RoadSegment seg;
                    seg.active = true;
                    seg.level = f.level;
                    seg.startNode = f.nodeId;
                    seg.endNode = pr.first.nodeId;

                    net.segments.add(seg);
                }
            }
        }

    private:
        std::shared_ptr<Field1F> mField1 = nullptr;

        void init(RoadNet &net, uint32_t origins)
        {
            auto newqueue = std::queue<lmcore::GrowthFront>();
            net.fronts.swap(newqueue);

            for (uint32_t i = 0u; i < origins; i++)
            {
                float x = net.random.nextFloat(-net.regionHalfWidth, net.regionHalfWidth);
                float y = net.random.nextFloat(-net.regionHalfHeight, net.regionHalfHeight);

                // TODO
                float doffset = 0.5f;
                float dx = net.random.nextFloat(-1.f, 1.f);
                float dy = net.random.nextFloat(-1.f, 1.f);
                f_normalize_2d_fast(dx, dy);
                dx *= doffset;
                dy *= doffset;

                float _x = x;
                float _y = y;
                f_normalize_2d_fast(_x, _y);

                dx += _x;
                dy += _y;
                f_normalize_2d_fast(dx, dy);

                float pdx = dx;
                float pdy = dy;
                float ndx = -dx;
                float ndy = -dy;

                float modifier = std::max(net.regionHalfHeight, net.regionHalfWidth);
                float ptx = pdx * 2.f * modifier + x;
                float pty = pdy * 2.f * modifier + y;
                float ntx = ndx * 2.f * modifier + x;
                float nty = ndy * 2.f * modifier + y;

                uint32_t idx = net.nodes.get_size();
                RoadNode node{.pos = GVec3f{x, y}};
                auto h = mField1->Sample(x, y);
                node.pos.z = h;

                net.nodes.add(node);

                GrowthFront front1;
                front1.dir = GVec2f{pdx, pdy};
                front1.length = 0.f;
                front1.level = 1;
                front1.nodeId = idx;
                front1.parentSegment = -1;
                front1.target = GVec2f{ptx, pty};

                GrowthFront front2;
                front2.dir = GVec2f{ndx, ndy};
                front2.length = 0.f;
                front2.level = 1;
                front2.nodeId = idx;
                front2.parentSegment = -1;
                front2.target = GVec2f{ntx, nty};

                net.fronts.push(front1);
                net.fronts.push(front2);
            }
        }

        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            auto dv = mField1->SampleDxyz(tv.x, tv.y, length, {tdx, tdy, 0.f}, 0.5);
            if (dv.x == 0.f && dv.y == 0.f)
                return {GrowthFront{.level = -1}, node};

            float d_mod = 0.3f;
            float dx = dv.x * (1.f - d_mod) + front.dir.x * d_mod;
            float dy = dv.y * (1.f - d_mod) + front.dir.y * d_mod;

            tv.x += dx * length;
            tv.y += dy * length;
            auto currentz = mField1->Sample(tv.x, tv.y);
            // todo
            if (currentz == 0.f)
                return {GrowthFront{.level = -1}, node};

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = currentz;

            front.length += length;

            return {front, node};
        }
    };

    class L2Growth
    {
    public:
        L2Growth(std::shared_ptr<Field1F> field1) : mField1(field1)
        {
            assert(field1 != nullptr);
        }

        ~L2Growth()
        {
        }

        void Grow(RoadNet &net, uint32_t steps, float length, uint32_t maxBranch)
        {
            init(net, steps, maxBranch);

            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = net.fronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = net.fronts.front();
                    net.fronts.pop();

                    //  if(f.length > 5.f)
                    //      continue;

                    RoadNode n = net.nodes[f.nodeId];
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, length);

                    // float drop_rate = 0.05f;
                    // bool drop = net.random.nextFloat(0.f, 1.f) < drop_rate;

                    if (pr.first.level < 0)
                        continue;

                    auto potentialfront = pr.first;
                    auto potentialnode = pr.second;

                    bool has_intersect = false;
                    int int_node_id = -1;
                    GVec3f newPos;

                    auto segidx = net.segments.getIndicesArray();
                    for (auto sidx : segidx)
                    {
                        auto &s = net.segments.get(sidx);
                        auto s0 = net.nodes[s.startNode].pos;
                        auto s1 = net.nodes[s.endNode].pos;
                        auto s2 = net.nodes[f.nodeId].pos;
                        auto s3 = pr.second.pos;

                        has_intersect = segmentsProperlyIntersect2D(s0, s1, s2, s3);
                        if (has_intersect)
                        {
                            int_node_id = s.startNode;
                            newPos.x = (s0.x + s1.x + s2.x + s3.x) / 4.f;
                            newPos.y = (s0.y + s1.y + s2.y + s3.y) / 4.f;
                            newPos.z = mField1->Sample(newPos.x, newPos.y);
                            break;
                        }
                    }

                    if (has_intersect)
                    {
                        pr.first.nodeId = int_node_id;
                        net.nodes[int_node_id].pos = newPos;
                    }
                    else
                    {
                        uint32_t nidx = net.nodes.get_size();
                        net.nodes.add(pr.second);
                        pr.first.nodeId = nidx;
                        net.fronts.push(pr.first);
                    }

                    RoadSegment seg;
                    seg.active = true;
                    seg.level = f.level;
                    seg.startNode = f.nodeId;
                    seg.endNode = pr.first.nodeId;

                    net.segments.add(seg);
                }
            }
        }

    private:
        std::shared_ptr<Field1F> mField1 = nullptr;

        void init(RoadNet &net, uint32_t steps, uint32_t maxbranch)
        {
            auto newqueue = std::queue<lmcore::GrowthFront>();
            net.fronts.swap(newqueue);

            uint32_t bcount = 0;
            uint32_t segcount = net.segments.get_size();
            float accept_thresh = 0.0f;
            int itercount = 0;

            std::vector<GVec2f> selected;
            float radius = 0.7f;
            float radiussq = radius * radius;

            while (bcount < maxbranch && itercount < 100000u)
            {
                itercount++;
                uint32_t sidx = net.random.nextInt(0, segcount - 1);
                auto seg = net.segments[sidx];
                auto snode = net.nodes[seg.startNode];
                auto enode = net.nodes[seg.endNode];

                float distsq = distanceSq2D({snode.pos.x, snode.pos.y}, {0.f, 0.f});
                float region = std::min(net.regionHalfHeight, net.regionHalfWidth);
                float filter = net.random.nextGaussianApprox(-region, region);
                filter *= filter;

                if (distsq > filter)
                    continue;

                float minradsq = radiussq;
                for (const auto &slc : selected)
                {
                    auto s = GVec2f{snode.pos.x, snode.pos.y};
                    minradsq = min(distanceSq2D(s, slc), minradsq);
                }

                if (net.random.nextFloat(0.f, 1.f) < accept_thresh || minradsq < radiussq)
                    continue;

                float sign = net.random.nextSignFloat();
                float nx = (enode.pos.x - snode.pos.x) * sign;
                float ny = (enode.pos.y - snode.pos.y) * sign;

                f_normalize_2d_fast(nx, ny);
                auto bnorm = GVec2f{.x = ny, .y = -nx};
                GrowthFront front;
                front.dir = bnorm;
                // todo
                front.target = GVec2f{.x = ny * 20.f, .y = -nx * 20.f};
                front.length = 0.f;
                front.level = 2;
                front.nodeId = seg.startNode;
                net.fronts.push(front);
                selected.push_back({snode.pos.x, snode.pos.y});
                bcount++;
            }
        }

        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            auto dv = mField1->SampleDxyz(tv.x, tv.y, length, {tdx, tdy, 0.f}, 0.5);
            if (dv.x == 0.f && dv.y == 0.f)
                return {GrowthFront{.level = -1}, node};

            float d_mod = 0.3f;
            float dx = dv.x * (1.f - d_mod) + front.dir.x * d_mod;
            float dy = dv.y * (1.f - d_mod) + front.dir.y * d_mod;

            tv.x += dx * length;
            tv.y += dy * length;
            auto currentz = mField1->Sample(tv.x, tv.y);
            // todo
            if (currentz == 0.f)
                return {GrowthFront{.level = -1}, node};

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = currentz;

            front.length += length;

            return {front, node};
        }
    };

    class L3Growth
    {
    public:
        L3Growth(std::shared_ptr<Field1F> field1) : mField1(field1)
        {
            assert(field1 != nullptr);
        }

        ~L3Growth()
        {
        }

        void Grow(RoadNet &net, uint32_t steps, float length, uint32_t maxBranch)
        {
            init(net, steps, maxBranch);

            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = net.fronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = net.fronts.front();
                    net.fronts.pop();

                    //  if(f.length > 5.f)
                    //      continue;

                    RoadNode n = net.nodes[f.nodeId];
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, length);

                    // float drop_rate = 0.05f;
                    // bool drop = net.random.nextFloat(0.f, 1.f) < drop_rate;

                    if (pr.first.level < 0)
                        continue;

                    auto potentialfront = pr.first;
                    auto potentialnode = pr.second;

                    bool has_intersect = false;
                    int int_node_id = -1;
                    GVec3f newPos;
                    auto segidx = net.segments.getIndicesArray();
                    for (auto sidx : segidx)
                    {
                        auto &s = net.segments.get(sidx);
                        auto s0 = net.nodes[s.startNode].pos;
                        auto s1 = net.nodes[s.endNode].pos;
                        auto s2 = net.nodes[f.nodeId].pos;
                        auto s3 = pr.second.pos;

                        has_intersect = segmentsProperlyIntersect2D(s0, s1, s2, s3);
                        if (has_intersect)
                        {
                            int_node_id = s.startNode;
                            newPos.x = (s0.x + s1.x + s2.x + s3.x) / 4.f;
                            newPos.y = (s0.y + s1.y + s2.y + s3.y) / 4.f;
                            newPos.z = mField1->Sample(newPos.x, newPos.y);
                            break;
                        }
                    }

                    if (has_intersect)
                    {
                        pr.first.nodeId = int_node_id;
                        net.nodes[int_node_id].pos = newPos;
                    }
                    else
                    {
                        uint32_t nidx = net.nodes.get_size();
                        net.nodes.add(pr.second);
                        pr.first.nodeId = nidx;
                        net.fronts.push(pr.first);
                    }

                    RoadSegment seg;
                    seg.active = true;
                    seg.level = f.level;
                    seg.startNode = f.nodeId;
                    seg.endNode = pr.first.nodeId;

                    net.segments.add(seg);
                }
            }
        }

    private:
        std::shared_ptr<Field1F> mField1 = nullptr;

        void init(RoadNet &net, uint32_t steps, uint32_t maxbranch)
        {
            auto newqueue = std::queue<lmcore::GrowthFront>();
            net.fronts.swap(newqueue);

            uint32_t bcount = 0;
            uint32_t segcount = net.segments.get_size();
            float accept_thresh = 0.0f;
            int itercount = 0;

            std::vector<GVec2f> selected;
            float radius = 0.6f;
            float radiussq = radius * radius;

            while (bcount < maxbranch && itercount < 100000u)
            {
                itercount++;
                uint32_t sidx = net.random.nextInt(0, segcount - 1);
                auto seg = net.segments[sidx];

                if (seg.level < 2)
                    continue;

                auto snode = net.nodes[seg.startNode];
                auto enode = net.nodes[seg.endNode];

                float distsq = distanceSq2D({snode.pos.x, snode.pos.y}, {0.f, 0.f});
                float region = std::min(net.regionHalfHeight, net.regionHalfWidth);
                float filter = net.random.nextGaussianApprox(-region, region);
                filter *= filter;

                if (distsq > filter)
                    continue;

                float minradsq = radiussq;
                for (const auto &slc : selected)
                {
                    auto s = GVec2f{snode.pos.x, snode.pos.y};
                    minradsq = min(distanceSq2D(s, slc), minradsq);
                }

                if (net.random.nextFloat(0.f, 1.f) < accept_thresh || minradsq < radiussq)
                    continue;

                float sign = net.random.nextSignFloat();
                float nx = (enode.pos.x - snode.pos.x) * sign;
                float ny = (enode.pos.y - snode.pos.y) * sign;

                f_normalize_2d_fast(nx, ny);
                auto bnorm = GVec2f{.x = ny, .y = -nx};
                GrowthFront front;
                front.dir = bnorm;
                // todo
                front.target = GVec2f{.x = ny * 20.f, .y = -nx * 20.f};
                front.length = 0.f;
                front.level = 3;
                front.nodeId = seg.startNode;
                net.fronts.push(front);
                selected.push_back({snode.pos.x, snode.pos.y});
                bcount++;
            }
        }

        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            auto dv = mField1->SampleDxyz(tv.x, tv.y, length, {tdx, tdy, 0.f}, 0.5);
            if (dv.x == 0.f && dv.y == 0.f)
                return {GrowthFront{.level = -1}, node};

            float d_mod = 0.6f;
            float dx = dv.x * (1.f - d_mod) + front.dir.x * d_mod;
            float dy = dv.y * (1.f - d_mod) + front.dir.y * d_mod;

            tv.x += dx * length;
            tv.y += dy * length;
            auto currentz = mField1->Sample(tv.x, tv.y);
            // todo
            if (currentz == 0.f)
                return {GrowthFront{.level = -1}, node};

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = currentz;

            front.length += length;

            return {front, node};
        }
    };

    class L4Growth
    {
    public:
        L4Growth(std::shared_ptr<Field1F> field1) : mField1(field1)
        {
            assert(field1 != nullptr);
        }

        ~L4Growth()
        {
        }

        void Grow(RoadNet &net, uint32_t steps, float length, uint32_t maxBranch)
        {
            init(net, steps, maxBranch);

            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = net.fronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = net.fronts.front();
                    net.fronts.pop();

                    //  if(f.length > 5.f)
                    //      continue;

                    RoadNode n = net.nodes[f.nodeId];
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, length);

                    // float drop_rate = 0.05f;
                    // bool drop = net.random.nextFloat(0.f, 1.f) < drop_rate;

                    if (pr.first.level < 0)
                        continue;

                    auto potentialfront = pr.first;
                    auto potentialnode = pr.second;

                    bool has_intersect = false;
                    int int_node_id = -1;
                    GVec3f newPos;
                    auto segidx = net.segments.getIndicesArray();
                    for (auto sidx : segidx)
                    {
                        auto &s = net.segments.get(sidx);
                        auto s0 = net.nodes[s.startNode].pos;
                        auto s1 = net.nodes[s.endNode].pos;
                        auto s2 = net.nodes[f.nodeId].pos;
                        auto s3 = pr.second.pos;

                        has_intersect = segmentsProperlyIntersect2D(s0, s1, s2, s3);
                        if (has_intersect)
                        {
                            int_node_id = s.startNode;
                            newPos.x = (s0.x + s1.x + s2.x + s3.x) / 4.f;
                            newPos.y = (s0.y + s1.y + s2.y + s3.y) / 4.f;
                            newPos.z = mField1->Sample(newPos.x, newPos.y);
                            break;
                        }
                    }

                    if (has_intersect)
                    {
                        pr.first.nodeId = int_node_id;
                        net.nodes[int_node_id].pos = newPos;
                    }
                    else
                    {
                        uint32_t nidx = net.nodes.get_size();
                        net.nodes.add(pr.second);
                        pr.first.nodeId = nidx;
                        net.fronts.push(pr.first);
                    }

                    RoadSegment seg;
                    seg.active = true;
                    seg.level = f.level;
                    seg.startNode = f.nodeId;
                    seg.endNode = pr.first.nodeId;

                    net.segments.add(seg);
                }
            }
        }

    private:
        std::shared_ptr<Field1F> mField1 = nullptr;

        void init(RoadNet &net, uint32_t steps, uint32_t maxbranch)
        {
            auto newqueue = std::queue<lmcore::GrowthFront>();
            net.fronts.swap(newqueue);

            uint32_t bcount = 0;
            uint32_t segcount = net.segments.get_size();
            float accept_thresh = 0.0f;
            int itercount = 0;

            std::vector<GVec2f> selected;
            float radius = 0.5f;
            float radiussq = radius * radius;

            while (bcount < maxbranch && itercount < 100000u)
            {
                itercount++;
                uint32_t sidx = net.random.nextInt(0, segcount - 1);
                auto seg = net.segments[sidx];

                if (seg.level < 3)
                    continue;

                auto snode = net.nodes[seg.startNode];
                auto enode = net.nodes[seg.endNode];

                float distsq = distanceSq2D({snode.pos.x, snode.pos.y}, {0.f, 0.f});
                float region = std::min(net.regionHalfHeight, net.regionHalfWidth);
                float filter = net.random.nextGaussianApprox(-region, region);
                filter *= filter;

                if (distsq > filter)
                    continue;

                float minradsq = radiussq;
                for (const auto &slc : selected)
                {
                    auto s = GVec2f{snode.pos.x, snode.pos.y};
                    minradsq = min(distanceSq2D(s, slc), minradsq);
                }

                if (net.random.nextFloat(0.f, 1.f) < accept_thresh || minradsq < radiussq)
                    continue;

                float sign = net.random.nextSignFloat();
                float nx = (enode.pos.x - snode.pos.x) * sign;
                float ny = (enode.pos.y - snode.pos.y) * sign;

                f_normalize_2d_fast(nx, ny);
                auto bnorm = GVec2f{.x = ny, .y = -nx};
                GrowthFront front;
                front.dir = bnorm;
                // todo
                front.target = GVec2f{.x = ny * 20.f, .y = -nx * 20.f};
                front.length = 0.f;
                front.level = 4;
                front.nodeId = seg.startNode;
                net.fronts.push(front);
                selected.push_back({snode.pos.x, snode.pos.y});
                bcount++;
            }
        }

        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            auto dv = mField1->SampleDxyz(tv.x, tv.y, length, {tdx, tdy, 0.f}, 0.5);
            if (dv.x == 0.f && dv.y == 0.f)
                return {GrowthFront{.level = -1}, node};

            float d_mod = 0.8f;
            float dx = dv.x * (1.f - d_mod) + front.dir.x * d_mod;
            float dy = dv.y * (1.f - d_mod) + front.dir.y * d_mod;

            tv.x += dx * length;
            tv.y += dy * length;
            auto currentz = mField1->Sample(tv.x, tv.y);
            // todo
            if (currentz == 0.f)
                return {GrowthFront{.level = -1}, node};

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = currentz;

            front.length += length;

            return {front, node};
        }
    };
}