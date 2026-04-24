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

        uint32_t get_full_size() const
        {
            return content.size();
        }

        uint32_t get_size() const
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
                content[idx] = t;
            }
            else
            {
                idx = indices.size();
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

        const std::vector<T> getArray() const
        {
            std::vector<T> array;

            for (auto &i : indices)
            {
                if (i.first)
                    array.push_back(content[i.second]);
            }

            return std::move(array);
        }

        const std::vector<uint32_t> getIndicesArray() const
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

    // enum class RoadLevel
    // {
    //     Main = 0,
    //     Secondary,

    // };

#define MAX_NODE_EDGE_COUNT 8u
    struct RoadNode
    {
        GVec3f pos;
        const static uint32_t sMaxEdgeCount = MAX_NODE_EDGE_COUNT;
        uint32_t edgeIndices[sMaxEdgeCount];
        uint32_t edgeCount = 0u;
        uint32_t halfedgeIndices[sMaxEdgeCount];
        uint32_t halfedgeCount = 0u;
    };

    struct RoadHalfEdge
    {
        int from;
        int to;
        int twin;
        int next = 0xFFFFFFFF;
        bool visited = false;
    };

    struct RoadSegment
    {
        int startNode;
        int endNode;
        int level;
        bool active;

        RoadHalfEdge hEdges[2];

        RoadHalfEdge &get_half_edge(int idx)
        {
            if (startNode == idx)
                return hEdges[0];
            if (endNode == idx)
                return hEdges[1];

            assert(false);
        }

        // void set_half_edge
    };

    struct Block
    {
        std::vector<GVec3f> pts;
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
        explicit RoadNet(float width, float height) : regionHalfWidth(width / 2.f), regionHalfHeight(height / 2.f) //, random(seed)
        {
        }

        float regionHalfWidth;
        float regionHalfHeight;

        // std::vector<RoadNode> nodes;
        // std::vector<RoadSegment> segments;

        SparseVector<RoadNode> nodes;
        SparseVector<RoadSegment> segments;
        std::vector<RoadHalfEdge> half_edges;
        std::vector<Block> blocks;

        std::queue<GrowthFront> fronts;
        // lmcore::Random random = lmcore::Random{1u};
    };

    class RoadNetOperator
    {
    public:
        const static uint32_t kInvalidU32 = 0xFFFFFFFF;

        RoadNetOperator(float width, float height) : mNet(width, height) {}
        const RoadNet &get() const
        {
            return mNet;
        }

        RoadNet &get()
        {
            return mNet;
        }

        const RoadNode &get_node(uint32_t idx) const
        {
            assert(mNet.nodes.is_valid(idx));
            return mNet.nodes[idx];
        }

        RoadNode &get_node(uint32_t idx)
        {
            assert(mNet.nodes.is_valid(idx));
            return mNet.nodes[idx];
        }

        const RoadSegment &get_edge(uint32_t idx) const
        {
            assert(mNet.segments.is_valid(idx));
            return mNet.segments[idx];
        }

        RoadSegment &get_edge(uint32_t idx)
        {
            assert(mNet.segments.is_valid(idx));
            return mNet.segments[idx];
        }

        const RoadSegment &get_edge(uint32_t idx1, uint32_t idx2) const
        {
            assert(check_node(idx1) && check_node(idx2));
            auto eidx = u_find_edge(idx1, idx2);
            return mNet.segments[eidx];
        }

        RoadSegment &get_edge(uint32_t idx1, uint32_t idx2)
        {
            assert(check_node(idx1) && check_node(idx2));
            auto eidx = u_find_edge(idx1, idx2);
            return mNet.segments[eidx];
        }

        uint32_t add_node(GVec3f pos)
        {
            RoadNode node;
            node.pos = pos;

            uint32_t idx = mNet.nodes.add(node);

            return idx;
        }

        uint32_t add_edge(uint32_t idx1, uint32_t idx2, int level)
        {
            assert(mNet.nodes.is_valid(idx1) && mNet.nodes.is_valid(idx2));
            auto &n1 = mNet.nodes[idx1];
            auto &n2 = mNet.nodes[idx2];
            assert(check_edge_count(idx1) && check_edge_count(idx2));
            assert(!has_edge(idx1, idx2));
            RoadSegment seg;
            seg.startNode = idx1;
            seg.endNode = idx2;
            seg.level = level;

            auto &he1 = seg.get_half_edge(idx1);
            auto &he2 = seg.get_half_edge(idx2);

            he1.from = idx1;
            he1.to = idx2;
            he2.from = idx2;
            he2.from = idx1;
            uint32_t idx = mNet.segments.add(seg);
            n1.edgeIndices[n1.edgeCount] = idx;
            n1.edgeCount++;
            n2.edgeIndices[n2.edgeCount] = idx;
            n2.edgeCount++;

            return idx;
        }

        void remove_node(uint32_t idx)
        {
            assert(check_node(idx));
            auto n = mNet.nodes[idx];
            uint32_t segcount = n.edgeCount;
            for (auto si = 0; si < segcount; si++)
            {
                auto seg = mNet.segments[si];
                auto n1 = seg.startNode;
                auto n2 = seg.endNode;

                uint32_t other = idx == n1 ? n2 : n1;
                auto &otherN = mNet.nodes[other];
                uint32_t eCount = 0;
                for (auto i = 0; i < otherN.edgeCount; i++)
                {
                    if (otherN.edgeIndices[i] != si)
                    {
                        otherN.edgeIndices[eCount++] = otherN.edgeIndices[i];
                    }
                }
                otherN.edgeCount = eCount;

                mNet.segments.remove(si);
            }

            mNet.nodes.remove(idx);
        }

        void remove_edge(uint32_t idx1, uint32_t idx2)
        {
            assert(check_node(idx1) && check_node(idx2));
            auto n1 = mNet.nodes[idx1];
            auto n2 = mNet.nodes[idx2];

            auto edge_index = u_find_edge(idx1, idx2);
            u_remove_edge_from_node_by_edge_index(idx1, edge_index);
            u_remove_edge_from_node_by_edge_index(idx2, edge_index);

            mNet.segments.remove(edge_index);
        }

        void remove_edge(uint32_t idx)
        {
            assert(check_edge(idx));
            auto &e = mNet.segments[idx];
            auto i1 = e.endNode;
            auto i2 = e.startNode;

            u_remove_edge_from_node_by_edge_index(i1, idx);
            u_remove_edge_from_node_by_edge_index(i2, idx);

            mNet.segments.remove(idx);
        }

        uint32_t insert_node(uint32_t ni1, uint32_t ni2, RoadNode node)
        {
            auto idx = mNet.nodes.add(node);
            auto e = get_edge(ni1, ni2);
            add_edge(idx, ni1, e.level);
            add_edge(idx, ni2, e.level);
            remove_edge(ni1, ni2);
            return idx;
        }

        // TODO handle multiple intersections
        bool propose_intersection(uint32_t startNodeIndex, GVec3f nextPos, uint32_t level, uint32_t &outidx)
        {
            uint32_t nidx = kInvalidU32;
            if (nextPos.x < -mNet.regionHalfWidth ||
                nextPos.x > mNet.regionHalfWidth ||
                nextPos.y < -mNet.regionHalfHeight ||
                nextPos.y > mNet.regionHalfHeight)
            {
                outidx = nidx;
                return false;
            }
            bool has_intersect = false;
            uint32_t edgeIndex = kInvalidU32;
            uint32_t sn1i = kInvalidU32;
            uint32_t sn0i = kInvalidU32;
            uint32_t sl = kInvalidU32;
            auto segidx = mNet.segments.getIndicesArray();
            GVec3f out;
            for (auto sidx : segidx)
            {
                auto &s = mNet.segments.get(sidx);
                auto s0 = mNet.nodes[s.startNode].pos;
                auto s1 = mNet.nodes[s.endNode].pos;

                auto s2 = mNet.nodes[startNodeIndex].pos;
                auto s3 = nextPos;

                if (equalVec3(s2, s0) || equalVec3(s2, s1))
                    continue;

                has_intersect = segmentIntersectionPoint2D(s0, s1, s2, s3, out);
                if (has_intersect)
                {
                    edgeIndex = sidx;
                    nextPos = out;
                    sn0i = s.startNode;
                    sn1i = s.endNode;
                    sl = s.level;
                    break;
                }
            }

            if (has_intersect)
            {
                auto e = get_edge(edgeIndex);
                auto sn = get_node(e.startNode);
                auto en = get_node(e.endNode);
                if (equalVec3(out, sn.pos))
                {
                    nidx = e.startNode;
                }
                else if (equalVec3(out, en.pos))
                {
                    nidx = e.endNode;
                }
                else
                {
                    nidx = add_node(nextPos);
                    remove_edge(edgeIndex);
                    add_edge(sn0i, nidx, sl);
                    add_edge(nidx, sn1i, sl);
                }
            }
            else
            {
                nidx = add_node(nextPos);
            }

            add_edge(startNodeIndex, nidx, level);
            outidx = nidx;
            return has_intersect;
        }

        void Build()
        {
            mNet.half_edges.clear();
            mNet.blocks.clear();

            auto segindices = mNet.segments.getIndicesArray();
            auto &nodes = mNet.nodes;
            auto &hedges = mNet.half_edges;

            for (auto si : segindices)
            {
                auto &seg = mNet.segments.get(si);
                auto n1i = seg.startNode;
                auto n2i = seg.endNode;
                auto &n1 = nodes.get(n1i);
                auto &n2 = nodes.get(n2i);

                RoadHalfEdge he1;
                RoadHalfEdge he2;

                he1.from = n1i;
                he1.to = n2i;
                he2.from = n2i;
                he2.to = n1i;

                auto he1i = hedges.size();
                auto he2i = he1i + 1;
                he1.twin = he2i;
                he2.twin = he1i;
                hedges.push_back(he1);
                hedges.push_back(he2);

                n1.halfedgeIndices[n1.halfedgeCount++] = he1i;
                n2.halfedgeIndices[n2.halfedgeCount++] = he2i;
            }

            // for (auto &he : mNet.half_edges)
            for (auto half_i = 0u; half_i < mNet.half_edges.size(); half_i++)
            {
                auto &he = mNet.half_edges[half_i];

                auto twinI = he.twin;
                auto &fromN = mNet.nodes.get(he.from);
                auto &toN = mNet.nodes.get(he.to);

                std::vector<std::pair<double, uint32_t>> hsort;
                for (auto i = 0; i < toN.halfedgeCount; i++)
                {
                    auto hi = toN.halfedgeIndices[i];
                    auto hto = mNet.half_edges[hi].to;
                    auto endpos = mNet.nodes.get(hto);
                    float dx = endpos.pos.x - toN.pos.x;
                    float dy = endpos.pos.y - toN.pos.y;
                    auto res = std::atan2(dy, dx);
                    hsort.push_back({res, hi});
                }

                std::sort(hsort.begin(), hsort.end(),
                          [](const std::pair<double, uint32_t> &a, const std::pair<double, uint32_t> &b)
                          {
                              return a.first < b.first;
                          });

                uint32_t target = 0xFFFFFFFF;
                // TODO tricky part
                if (toN.halfedgeCount == 1)
                    continue;
                for (auto i = 0; i < toN.halfedgeCount; i++)
                {
                    toN.halfedgeIndices[i] = hsort[i].second;
                    if (he.twin == hsort[i].second)
                    {
                        target = (i + 1) % toN.halfedgeCount;
                    }
                }
                if (target != 0xFFFFFFFF)
                    he.next = toN.halfedgeIndices[target];
            }

            for (auto half_i = 0u; half_i < mNet.half_edges.size(); half_i++)
            {
                auto &he = mNet.half_edges[half_i];
                if (he.visited == true)
                    continue;

                he.visited = true;

                uint32_t next = he.next;
                Block block;
                block.pts.push_back(nodes.get(he.from).pos);
                bool closed = false;
                while (true)
                {
                    if (next == 0xFFFFFFFF)
                        break;
                    if (next == half_i)
                    {
                        closed = true;
                        break;
                    }
                    auto &nhe = mNet.half_edges[next];
                    block.pts.push_back(nodes.get(nhe.from).pos);
                    nhe.visited = true;
                    next = nhe.next;
                }

                if (closed)
                    mNet.blocks.push_back(block);
            }
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

        const uint32_t u_find_edge(uint32_t ni1, uint32_t ni2) const
        {
            auto &n = get_node(ni1);
            uint32_t ec = n.edgeCount;
            for (auto i = 0; i < ec; i++)
            {
                auto &s = mNet.segments[n.edgeIndices[i]];
                if (ni2 == s.startNode || ni2 == s.endNode)
                    return n.edgeIndices[i];
            }
            assert(false);
            return 0u;
        }

        const uint32_t u_find_edge_in_node_by_node_index(uint32_t ni1, uint32_t ni2) const
        {
            auto &n = get_node(ni1);
            uint32_t ec = n.edgeCount;
            for (auto i = 0; i < ec; i++)
            {
                auto &s = mNet.segments[n.edgeIndices[i]];
                if (ni2 == s.startNode || ni2 == s.endNode)
                    return i;
            }
            assert(false);
            return 0u;
        }

        const uint32_t u_find_edge_in_node_by_edge_index(uint32_t ni, uint32_t ei) const
        {
            auto &n = get_node(ni);

            uint32_t ec = n.edgeCount;
            for (auto i = 0; i < ec; i++)
            {
                if (n.edgeIndices[i] == ei)
                    return i;
            }
            assert(false);
            return 0u;
        }

        void u_remove_edge_index(uint32_t ni, uint32_t ei)
        {
            auto &n = get_node(ni);
            --n.edgeCount;
            if (n.edgeCount > 0)
            {
                n.edgeIndices[ei] = n.edgeIndices[n.edgeCount];
            }
        }

        void u_remove_edge_from_node_by_node_index(uint32_t ni1, uint32_t ni2)
        {
            uint32_t nei = u_find_edge_in_node_by_node_index(ni1, ni2);
            u_remove_edge_index(ni1, nei);
        }

        void u_remove_edge_from_node_by_edge_index(uint32_t ni, uint32_t ei)
        {
            auto &e = mNet.segments[ei];
            auto &n = mNet.nodes[ni];
            uint32_t nei = u_find_edge_in_node_by_edge_index(ni, ei);
            u_remove_edge_index(ni, nei);
        }

        RoadNet mNet;
    };

    struct FieldsArray
    {
        std::shared_ptr<Field1F> height_field = nullptr;
    };

    class RoadGenerationPolicy
    {
    public:
        explicit RoadGenerationPolicy(FieldsArray fields) : mFields(fields) {}
        virtual ~RoadGenerationPolicy() {}
        virtual void Apply(RoadNetOperator &net, uint32_t steps, float size) = 0;

    protected:
        // todo
        std::vector<uint32_t> segment_filter(RoadNetOperator &net, uint32_t level)
        {
            auto indice = net.get().segments.getIndicesArray();
            std::vector<uint32_t> filter;
            for (auto i : indice)
            {
                if (net.get_edge(i).level == level)
                    filter.push_back(i);
            }

            return filter;
        }

    protected:
        FieldsArray mFields;
        std::queue<GrowthFront> mFronts;
        lmcore::Random mRandom;
    };

    class AltitudeSamplePolicy : public RoadGenerationPolicy
    {
    public:
        explicit AltitudeSamplePolicy(FieldsArray fields) : RoadGenerationPolicy(fields) {}
        virtual void Apply(RoadNetOperator &net, uint32_t steps = 0u, float size = 0.f)
        {
            auto &netData = net.get();
            auto ia = netData.nodes.getIndicesArray();
            for (auto i : ia)
            {
                auto &p = netData.nodes[i].pos;
                p.z = mFields.height_field->Sample(p.x, p.y);
            }
        }
    };

    class DefaultMainRoadPolicy : public RoadGenerationPolicy
    {
    public:
        explicit DefaultMainRoadPolicy(FieldsArray fields) : RoadGenerationPolicy(fields) {}
        virtual void Apply(RoadNetOperator &net, uint32_t steps, float size)
        {
            float h = net.get().regionHalfHeight;
            float w = net.get().regionHalfWidth;
            float rs = 0.6f;
            float px = mRandom.nextFloat(-w * rs, w * rs);
            float py = mRandom.nextFloat(-h * rs, h * rs);
            float pz = mFields.height_field->Sample(px, py);
            GVec3f pos = {px, py, pz};
            auto ni = net.add_node(pos);

            float dx = mRandom.nextFloat(-1.f, 1.f);
            float dy = mRandom.nextFloat(-1.f, 1.f);
            f_normalize_2d_fast(dx, dy);

            GrowthFront front0;
            front0.level = 1u;
            front0.nodeId = ni;
            front0.dir = {dx, dy};

            GrowthFront front1;
            front1.level = 1u;
            front1.nodeId = ni;
            front1.dir = {-dx, -dy};

            GrowthFront front2;
            front2.level = 1u;
            front2.nodeId = ni;
            front2.dir = {dy, -dx};

            GrowthFront front3;
            front3.level = 1u;
            front3.nodeId = ni;
            front3.dir = {-dy, dx};

            mFronts.push(front0);
            mFronts.push(front1);
            mFronts.push(front2);
            mFronts.push(front3);

            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = mFronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = mFronts.front();
                    mFronts.pop();
                    auto &netdata = net.get();
                    RoadNode n = net.get_node(f.nodeId);
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, size);

                    if (pr.first.level < 0)
                        continue;

                    uint32_t idx;
                    auto hasi = net.propose_intersection(f.nodeId, pr.second.pos, pr.first.level, idx);

                    if (idx != net.kInvalidU32)
                    {
                        pr.first.nodeId = idx;
                        mFronts.push(pr.first);
                    }
                }
            }
        }

    private:
        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            float dx = front.dir.x;
            float dy = front.dir.y;

            tv.x += dx * length;
            tv.y += dy * length;

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = 0.f;

            front.length += length;

            return {front, node};
        }
    };

    class DefaultSecondaryRoadPolicy : public RoadGenerationPolicy
    {
    public:
        DefaultSecondaryRoadPolicy(FieldsArray fields) : RoadGenerationPolicy(fields) {}
        void SetOriginCount(uint32_t count)
        {
            mOriginCount = count;
        }
        virtual void Apply(RoadNetOperator &net, uint32_t steps, float size)
        {
            auto filter = segment_filter(net, 1);
            auto filter_size = filter.size();
            uint32_t count = 0u;
            uint32_t limit = 0u;
            std::vector<uint32_t> picked;
            while (count < mOriginCount && limit < 100000u)
            {
                limit++;
                uint32_t idx = mRandom.nextGaussianApprox(0.0, 1.0) * filter_size;
                bool found = false;
                for (auto i : picked)
                {
                    if (i == idx)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    picked.push_back(idx);
                    count++;
                }
            }

            for (auto p : picked)
            {
                auto &e = net.get_edge(p);
                auto &sn = net.get_node(e.startNode);
                auto &en = net.get_node(e.endNode);

                float dx = en.pos.x - sn.pos.x;
                float dy = en.pos.y - sn.pos.y;
                auto dir = perpendicularLeft(dx, dy);

                dir = normalize(dir) * mRandom.nextSign();
                GrowthFront front;
                front.dir = dir;
                front.length = size;
                front.level = 2;
                front.nodeId = e.endNode;
                mFronts.push(front);
            }

            for (uint32_t i = 0u; i < steps; i++)
            {
                uint32_t q_size = mFronts.size();
                for (uint32_t qi = 0u; qi < q_size; qi++)
                {
                    GrowthFront f = mFronts.front();
                    mFronts.pop();

                    RoadNode n = net.get_node(f.nodeId);
                    std::pair<GrowthFront, RoadNode> pr = grow(f, n, size);

                    if (pr.first.level < 0)
                        continue;

                    uint32_t idx;
                    auto hasi = net.propose_intersection(f.nodeId, pr.second.pos, pr.first.level, idx);
                    int rate = 1.0;
                    if (hasi)
                    {
                        rate = mRandom.nextFloat(0.f, 1.f);
                    }

                    if (idx != net.kInvalidU32 && rate > 0.15f)
                    {
                        pr.first.nodeId = idx;
                        mFronts.push(pr.first);
                    }
                }
            }
        }

    private:
        std::pair<GrowthFront, RoadNode> grow(GrowthFront front, RoadNode node, float length)
        {
            float move_u = node.pos.x;
            float move_v = node.pos.y;

            float tdx = front.target.x - move_u;
            float tdy = front.target.y - move_v;

            lmcore::GVec2f tv = {move_u, move_v};

            float dx = front.dir.x;
            float dy = front.dir.y;

            tv.x += dx * length;
            tv.y += dy * length;

            node.pos.x = tv.x;
            node.pos.y = tv.y;
            node.pos.z = 0.f;

            front.length += length;

            return {front, node};
        }

    private:
        uint32_t mOriginCount = 16u;
    };

    class BlockGenerator
    {
    public:
        BlockGenerator() {}
        void Generate(RoadNetOperator &netopt)
        {
            auto &segments = netopt.get().segments;
            auto eindices = segments.getIndicesArray();
            auto &nodes = netopt.get().nodes;
            auto nindices = nodes.getIndicesArray();

            std::vector<Block> blocks;
            for (auto i : nindices)
            {
                auto &n = nodes.get(i);

                for (uint32_t j = 0; j < n.edgeCount; i++)
                {
                    Block block;
                    block.pts.push_back(n.pos);

                    auto ei = n.edgeIndices[j];
                    auto &s = segments.get(ei);

                    uint32_t to = i == s.endNode ? s.startNode : s.endNode;
                    auto &this_half = s.get_half_edge(i);
                    if (this_half.visited)
                        continue;

                    while (to != j && to != RoadNetOperator::kInvalidU32)
                    {
                        // auto &to_n = nodes.get(to);
                        // float actan_value;
                        // std::
                        // for(uint32_t to_edge_i = 0; to_edge_i<to_n.edgeCount; to_edge_i++)
                        // {
                        //     auto& to_edge = segments.get(to_edge_i);
                        //     auto& to_h_edge = to_edge.get_half_edge(to);
                        // }
                    }
                }
            }
        }
    };
}