#include "nifUtil.h"

namespace Mus::nif {
    namespace debug {
        bool visitObjects(RE::NiAVObject* a_object, std::function<bool(RE::NiAVObject*, int)> a_func, int depth) {
            if (!a_object) return true;

            auto result = a_func(a_object, depth);
            if (!result)
                return result;

            result = true;
            auto node = a_object->AsNode();
            if (node) {
                for (auto& child : node->GetChildren()) {
                    result = visitObjects(child.get(), a_func, depth + 1);
                    if (!result) {
                        break;
                    }
                }
            }

            return result;
        }

        bool printStuff(RE::NiAVObject* a_object, int depth) {
            if (!a_object) return true;
            std::string sss = spaces(depth);
            const char* ss = sss.c_str();
            logger::info("{}avObj Name = {}, RTTI = {}", ss, a_object->name.c_str(), a_object->GetRTTI() ? a_object->GetRTTI()->name : "");

            auto node = a_object->AsNode();
            if (node) {
                logger::info("{}node {}, RTTI {}", ss, node->name.c_str(), node->GetRTTI() ? a_object->GetRTTI()->name : "");
            }
            return true;
        }

        RE::BSVisit::BSVisitControl printGeometry(RE::BSGeometry* a_geometry) {
            if (!a_geometry) return RE::BSVisit::BSVisitControl::kContinue;
            logger::info("avObj Name = {}, RTTI = {}", a_geometry->name.c_str(), a_geometry->GetRTTI() ? a_geometry->GetRTTI()->name : "");
            return RE::BSVisit::BSVisitControl::kContinue;
        }

        RE::BSVisit::BSVisitControl printAVObject(RE::NiAVObject* a_object) {
            if (!a_object) return RE::BSVisit::BSVisitControl::kContinue;
            logger::info("avObj Name = {}, RTTI = {}", a_object->name.c_str(), a_object->GetRTTI() ? a_object->GetRTTI()->name : "");

            auto node = a_object->AsNode();
            if (node) {
                logger::info("node {}, RTTI {}", node->name.c_str(), node->GetRTTI() ? a_object->GetRTTI()->name : "");
            }
            return RE::BSVisit::BSVisitControl::kContinue;
        }

        void printObjTree(RE::NiAVObject* obj)
        {
            logger::info("print geometry tree");
            RE::BSVisit::TraverseScenegraphGeometries(obj, printGeometry);
            //logger::info("print avobject tree");
            //RE::BSVisit::TraverseScenegraphObjects(obj, printAVObject);
        }
    }

    void TaskupdateNode::Run() {
        RE::NiUpdateData ctx;
        ctx.flags = RE::NiUpdateData::Flag::kDirty;
        ctx.time = 0;
        obj->UpdateWorldData(&ctx);
    }

    RE::NiNode* addParentToNode(RE::NiNode* node, const char* name)
    {
        if (!node)
            return nullptr;

        auto parent = node->parent;
        auto newParent = RE::NiNode::Create(1);
        node->IncRefCount();
        if (parent)
        {
            parent->DetachChild2(node);
            parent->AttachChild(newParent, true);
        }
        newParent->AttachChild(node, true);
        setNiNodeName(newParent, name);
        node->DecRefCount();
        return newParent;
    }

    RE::NiNode* addChildToNode(RE::NiNode* node, const char* name)
    {
        if (!node)
            return nullptr;

        auto& childlist = node->GetChildren();
        std::vector<RE::NiNode*> childs;
        for (auto& obj : childlist)
        {
            if (obj.get())
            {
                if (RE::NiNode* objnode = obj.get()->AsNode(); objnode)
                    childs.emplace_back(objnode);
            }
        }

        auto newChild = RE::NiNode::Create(1);
        if (!newChild)
            return nullptr;

        if (childs.size() > 0)
        {
            bool Attached = false;
            for (std::size_t i = 0; i < childs.size(); i++)
            {
                RE::NiNode* child = childs[i];
                if (!child)
                    continue;
                child->IncRefCount();
                node->DetachChild2(child);
                if (!Attached)
                    node->AttachChild(newChild, true);
                newChild->AttachChild(child, true);
                if (!Attached)
                    setNiNodeName(newChild, name);
                child->DecRefCount();
                Attached = true;
            }
        }
        else
        {
            node->AttachChild(newChild, true);
            setNiNodeName(newChild, name);
        }
        return newChild;
    }

    RE::NiMatrix3 SetMatrixByEntry(float a_entry00, float a_entry01, float a_entry02,
        float a_entry10, float a_entry11, float a_entry12,
        float a_entry20, float a_entry21, float a_entry22)
    {
        RE::NiMatrix3 matrix;

        matrix.entry[0][0] = a_entry00;
        matrix.entry[0][1] = a_entry01;
        matrix.entry[0][2] = a_entry02;
        matrix.entry[1][0] = a_entry10;
        matrix.entry[1][1] = a_entry11;
        matrix.entry[1][2] = a_entry12;
        matrix.entry[2][0] = a_entry20;
        matrix.entry[2][1] = a_entry21;
        matrix.entry[2][2] = a_entry22;

        return matrix;
    }

    RE::NiPoint3 GetEulerAngles(RE::NiMatrix3 a_angles)
    {
        RE::NiPoint3 a_point;
        if (a_angles.entry[1][0] > 0.998) { // singularity at north pole
            a_point.x = 0;
            a_point.y = atan2(a_angles.entry[0][2], a_angles.entry[2][2]);
            a_point.z = MATH_PI / 2;
        }
        else if (a_angles.entry[1][0] < -0.998) { // singularity at south pole
            a_point.x = 0;
            a_point.y = atan2(a_angles.entry[0][2], a_angles.entry[2][2]);
            a_point.z = -MATH_PI / 2;
        }
        else {
            a_point.x = atan2(-a_angles.entry[1][2], a_angles.entry[1][1]);
            a_point.y = atan2(-a_angles.entry[2][0], a_angles.entry[0][0]);
            a_point.z = asin(a_angles.entry[1][0]);
        }
        return a_point;
    }

    RE::NiMatrix3 SetEulerAngles(float a_point_x, float a_point_y, float a_point_z)
    {
        RE::NiMatrix3 a_angles;
        double cb = cos(a_point_x);
        double sb = sin(a_point_x);
        double ch = cos(a_point_y);
        double sh = sin(a_point_y);
        double ca = cos(a_point_z);
        double sa = sin(a_point_z);

        a_angles.entry[0][0] = ch * ca;
        a_angles.entry[0][1] = sh * sb - ch * sa * cb;
        a_angles.entry[0][2] = ch * sa * sb + sh * cb;
        a_angles.entry[1][0] = sa;
        a_angles.entry[1][1] = ca * cb;
        a_angles.entry[1][2] = -ca * sb;
        a_angles.entry[2][0] = -sh * ca;
        a_angles.entry[2][1] = sh * sa * cb + ch * sb;
        a_angles.entry[2][2] = -sh * sa * sb + ch * cb;
        return a_angles;
    }

    RE::NiMatrix3 SetEulerAngles(RE::NiPoint3 a_point)
    {
        return SetEulerAngles(a_point.x, a_point.y, a_point.z);
    }
}