#include "pinocchio/spatial/se3.hpp"
#include <graaf/graaf.hpp>
#include <Eigen/Dense>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <chrono>
#include <vector>

// Placeholder for Pinocchio integration
namespace pinocchio { using SE3 = int; } // Mock for example

class Pose {
public:
    Eigen::Matrix3d rotation = Eigen::Matrix3d::Identity();
    Eigen::Vector3d position{0.0, 0.0, 0.5}; // Fixed syntax

    pinocchio::SE3 to_SE3() {
        // Bridge to Pinocchio SE3 here
        return 0; 
    }
    
    static Pose from_SE3(pinocchio::SE3 pose) {
        return Pose{};
    }
};

struct Transform {
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    std::string parent_frame;
    std::string child_frame;
    Pose pose;
};

class TransformBuffer{

    std::vector<Transform> transforms;

public:
    void add_transform(const Transform& transform) {
        transforms.push_back(transform);
    }

    const std::vector<Transform>& get_transforms() const {
        return transforms;
    }

    const Transform* get_transform_at_time(const std::chrono::time_point<std::chrono::steady_clock>& timestamp, bool interpolate = false) const {
        for (const auto& transform : transforms) {
            if (transform.timestamp == timestamp) {
                return &transform;
            }
        }
        if (interpolate) {
            // Implement interpolation logic here if needed
        }

        return nullptr; // Not found
    }
};

struct Entity {
    std::string id;
    std::string mesh_path; // Your mesh data / placeholder
};

class Graph {
private:
    // Graph stores Entity as vertex data and Transform as edge data
    graaf::directed_graph<Entity, TransformBuffer> graph;
    
    // Maps string names/IDs to graaf's internal numeric vertex handles
    std::unordered_map<std::string, graaf::vertex_id_t> name_to_id;

public:
    void add_entity(const Entity& entity) {
        // add_vertex returns a graaf::vertex_id_t (size_t)
        graaf::vertex_id_t vertex_id = graph.add_vertex(entity);
        name_to_id[entity.id] = vertex_id;
    }

    void add_transform(const Transform& transform) {
        auto parent_it = name_to_id.find(transform.parent_frame);
        auto child_it = name_to_id.find(transform.child_frame);

        if (parent_it == name_to_id.end() || child_it == name_to_id.end()) {
            throw std::runtime_error("Parent or child frame not found in graph!");
        }

        // Pass the actual Transform struct as the edge payload using graaf IDs
        TransformBuffer buffer;
        buffer.add_transform(transform);
        graph.add_edge(parent_it->second, child_it->second, buffer);
    }

    void update_transform(const Transform& transform) {
        auto parent_it = name_to_id.find(transform.parent_frame);
        auto child_it = name_to_id.find(transform.child_frame);

        if (parent_it == name_to_id.end() || child_it == name_to_id.end()) {
            throw std::runtime_error("Parent or child frame not found in graph!");
        }

        auto parent_id = parent_it->second;
        auto child_id = child_it->second;

        auto& edge = graph.get_edge(parent_id, child_id);
        edge.add_transform(transform);
    }

// 1. Calculate the absolute pose of ANY frame relative to the root (world)
    pinocchio::SE3 get_world_pose(const std::string& frame) {
        std::vector<std::string> path_to_root;
        std::string current = frame;

        // Walk up the tree to the root frame
        while (parent_map.find(current) != parent_map.end()) {
            path_to_root.push_back(current);
            current = parent_map[current];
        }
        path_to_root.push_back(current); // Add the root itself

        // Accumulate transforms by multiplying them from the root going downwards
        pinocchio::SE3 world_pose = pinocchio::SE3::Identity();
        
        for (int i = path_to_root.size() - 1; i > 0; --i) {
            std::string parent = path_to_root[i];
            std::string child = path_to_root[i-1];
            
            auto parent_id = name_to_id[parent];
            auto child_id = name_to_id[child];
            
            const auto& edge = graph.get_edge(parent_id, child_id);
            
            // Spatial multiplication: T_accumulated = T_accumulated * T_next
            world_pose = world_pose * edge.pose.to_SE3(); 
        }
        return world_pose;
    }

    // 2. The Holy Grail: Lookup transform between ANY two arbitrary frames
    pinocchio::SE3 lookup_transform(const std::string& source_frame, const std::string& target_frame) {
        // Get absolute positions of both in the world
        pinocchio::SE3 T_world_source = get_world_pose(source_frame);
        pinocchio::SE3 T_world_target = get_world_pose(target_frame);

        // Compute the relative transform using Pinocchio's group inverse
        return T_world_source.inverse() * T_world_target;
    }
};

int main() {
    Graph graph;

    // Example usage
    Entity world{"world", "world_mesh"};
    Entity robot{"robot", "robot_mesh"};

    graph.add_entity(world);
    graph.add_entity(robot);

    Transform t{"world", "robot", pinocchio::SE3::Identity()};
    graph.add_transform(t);

    pinocchio::SE3 T = graph.lookup_transform("world", "robot");
    std::cout << "Transform from world to robot: " << T.translation().transpose() << std::endl;

    return 0;
}

