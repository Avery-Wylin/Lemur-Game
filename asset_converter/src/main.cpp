#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <queue>
#include <regex>
#include <algorithm>

#include <cglm/cglm.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::vector;
using std::string;
using std::pair;

const uint8_t MAX_JOINTS = 100;

void aiMat4x4tomat4(const aiMatrix4x4 source, mat4& dest)
{
    dest[0][0] = source.a1;
    dest[0][1] = source.b1;
    dest[0][2] = source.c1;
    dest[0][3] = source.d1;
    dest[1][0] = source.a2;
    dest[1][1] = source.b2;
    dest[1][2] = source.c2;
    dest[1][3] = source.d2;
    dest[2][0] = source.a3;
    dest[2][1] = source.b3;
    dest[2][2] = source.c3;
    dest[2][3] = source.d3;
    dest[3][0] = source.a4;
    dest[3][1] = source.b4;
    dest[3][2] = source.c4;
    dest[3][3] = source.d4;
}

void aiQuattoversor(const aiQuaternion& source, versor& dest){
    dest[0] = source.x;
    dest[1] = source.y;
    dest[2] = source.z;
    dest[3] = source.w;
}

struct MeshData {
    uint8_t
    posCount = 0,
    normCount = 0,
    uvCount = 0,
    colCount = 0,
    weightCount = 0,
    jointCount = 0;
    uint32_t vertexCount = 0;
    uint32_t faceCount = 0;
    string name;
    vector<uint32_t> index;
    vector<float> pos;
    vector<float> norm;
    vector<float> uv;
    vector<float> col;
    vector<float> weight;
    vector<uint8_t> joint;
    mat4 transform = GLM_MAT4_IDENTITY_INIT;
};

struct Joint{
    string name;
    vector<uint8_t> children;
    uint8_t parent = 0;
    mat4 local = GLM_MAT4_IDENTITY_INIT;
    mat4 rest = GLM_MAT4_IDENTITY_INIT;

    Joint(){}
    void printData() {
        std::cout << name;
        std::cout << " Parent: " << ( uint )parent;
        std::cout << " Children:";
        for( uint8_t &cid : children )
            std::cout << " " << ( uint )cid;
        vec3 pos = {};
        versor rot = {};
        glm_mat4_quat( rest, rot );
        glm_vec3_copy( rest[3], pos );
        std::cout << " Pos: " << pos[0] << " " << pos[1] << " " << pos[2];
        std::cout << " Rot: " << rot[0] << " " << rot[1] << " " << rot[2] << " " << rot[3];
        std::cout << std::endl;
    }
};

struct poskey{
    vec3 val;
    float t;
};

struct rotkey{
    versor val;
    float t;
};


struct AnimationData;

struct ArmatureData {
    string name;
    std::unordered_map<string, uint8_t> joint_names;
    vector<Joint> joints;
    vector<AnimationData> animations;

    void printData() {
        std::cout << "Armature: " << name << std::endl;
        std::cout << "Joints:" << std::endl;
        for(uint i = 0; i < joints.size(); ++i){
            std::cout << "Joint " << i << " : ";
            joints[i].printData();
        }
    }
};

struct Channel{
    constexpr static float LERP_ERROR = 0.01f;
    vector<rotkey> rotation;
    vector<poskey> position;
    uint8_t joint;

    void fromAiChannel(ArmatureData &armature, aiNodeAnim *ai_channel, float tps ){
        poskey pos;
        joint =  armature.joint_names[(string)std::string_view(ai_channel->mNodeName.C_Str())];

        printf("Joint: %u",joint);
        printf("\nPosition:");
        for( uint i = 0; i < ai_channel->mNumPositionKeys; ++i ) {
            pos.t = ai_channel->mPositionKeys[i].mTime/tps;
            glm_vec3_copy( reinterpret_cast<vec3 &>( ai_channel->mPositionKeys[i].mValue ), pos.val );
            position.push_back( pos );
            printf(" {[%.2f] %.2f %.2f %.2f }",pos.t,pos.val[0],pos.val[1],pos.val[2]);
        }

        printf("\nRotation:");

        rotkey rot;
        for( uint i = 0; i < ai_channel->mNumRotationKeys; ++i ) {
            rot.t = ai_channel->mRotationKeys[i].mTime/tps;
            aiQuattoversor( ai_channel->mRotationKeys[i].mValue, rot.val );
            rotation.push_back( rot );
            printf(" {[%.2f] %.2f %.2f %.2f %.2f }",rot.t,rot.val[0],rot.val[1],rot.val[2],rot.val[3]);
        }
        printf("\n");
    }
};

struct AnimationData{
    string name;
    vector<Channel> channels;
    float duration = 0;
    void fromAiAnimation(ArmatureData &armature, aiAnimation *ai_animation){
        ai_animation->mTicksPerSecond;
        channels.resize(ai_animation->mNumChannels);
        for(uint i = 0; i < ai_animation->mNumChannels; ++i){
            channels[i].fromAiChannel(armature, ai_animation->mChannels[i], ai_animation->mTicksPerSecond);
        }

        duration = 0;
        for(Channel c:channels){
            duration = fmax(duration,c.position.back().t);
            duration = fmax(duration,c.rotation.back().t);
        }
    }
};


// Function Declarations
void readHierarchy(aiNode *root);
void readJointHierarchy(aiNode *root);
void readAnimationData( aiAnimation *ai_animation );
void readMeshData( aiMesh *mesh );

void exportMeshData( MeshData *mesh );
void exportAnimationData(AnimationData *animation);
void exportArmatureData(ArmatureData *armature);


// Variables
std::unordered_map<string, MeshData> meshMap;
std::unordered_map<string, ArmatureData> armatureMap;
const aiScene *scene;


int main( int argc, char **argv ) {
    if(argc != 2){
        std::cout << "No file was specified or wrong format" << std::endl;
        exit(0);
    }
    string arg_filename;
    arg_filename = std::string_view(argv[1]);

    string filename = "../src_asset/"+arg_filename+".glb";

    // Create an instance of the Importer class
    Assimp::Importer importer;

    scene = importer.ReadFile( filename,
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_LimitBoneWeights

        );

    if( nullptr == scene ) {
        std::cout << "Error: " << importer.GetErrorString() << std::endl;
        return 1;
    }

    // Print number of meshes and animations found
    std::cout << "Animations: " << scene->mNumAnimations << std::endl;
    std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

    // Read armatures from the hierarchy
    readHierarchy(scene->mRootNode);

    // Read animation data
    for(uint i = 0; i < scene->mNumAnimations; ++i){
        readAnimationData(scene->mAnimations[i]);
    }

    // Read the data for each mesh, this requires the armature hierarchy for joints
    for( uint i = 0 ; i < scene->mNumMeshes; ++i ) {
        readMeshData( scene->mMeshes[i] );
    }

    // Export Mesh Data to PLY files
    for( auto &value : meshMap ) {
        exportMeshData( &value.second );
    }

    for(auto &value : armatureMap){
        exportArmatureData(&value.second);
    }

    return 0;
}

void readMeshData( aiMesh *aimesh ) {
    static std::regex numeric_suffix("[a-zA-Z0-9_]*\\.[0-9]{3}");
    MeshData &mesh = meshMap[string( aimesh->mName.C_Str() )];

    mesh.vertexCount = aimesh->mNumVertices;
    mesh.faceCount = aimesh->mNumFaces;
    mesh.name = std::string_view(aimesh->mName.C_Str());
    std::cout<< "Mesh: " << mesh.name << std::endl;

    // Remove affix blender places on meshes with the same name as a node
    // The affix interferes with linking meshes with joint data to an armature
    if(std::regex_match(mesh.name,numeric_suffix)){
        mesh.name.erase(mesh.name.length()-4,4);
    }

    // Position
    mesh.posCount = 3;
    for( uint i = 0; i < aimesh->mNumVertices; ++i ) {
        mesh.pos.push_back( aimesh->mVertices[i].x );
        mesh.pos.push_back( aimesh->mVertices[i].y );
        mesh.pos.push_back( aimesh->mVertices[i].z );
        vec3 &v = reinterpret_cast<vec3 &>( mesh.pos[mesh.pos.size() - 3] );
        glm_mat4_mulv3( mesh.transform, v, 1, v );
    }


    // Normal
    if( aimesh->HasNormals() ) {
        mesh.normCount = 3;
        for( uint i = 0; i < aimesh->mNumVertices; ++i ) {
            mesh.norm.push_back( aimesh->mNormals[i].x );
            mesh.norm.push_back( aimesh->mNormals[i].y );
            mesh.norm.push_back( aimesh->mNormals[i].z );
            vec3 &v = reinterpret_cast<vec3 &>( mesh.norm[mesh.norm.size() - 3] );
            glm_mat4_mulv3( mesh.transform, v, 0, v );
        }
    }


    // Color
    if( aimesh->HasVertexColors( 0 ) ) {
        mesh.colCount = 3;
        // Convert read colors to sRGB since gltf colors are linear
        for( uint i = 0; i < aimesh->mNumVertices; ++i ) {
            mesh.col.push_back( (float)pow(aimesh->mColors[0][i].r,1/2.2f) );
            mesh.col.push_back( (float)pow(aimesh->mColors[0][i].g,1/2.2f) );
            mesh.col.push_back( (float)pow(aimesh->mColors[0][i].b,1/2.2f) );
        }
    }

    // UV
    if( aimesh->HasTextureCoords( 0 ) ) {
        mesh.uvCount = 2;
        for( uint i = 0; i < aimesh->mNumVertices; ++i ) {
            mesh.uv.push_back( aimesh->mTextureCoords[0][i].x );
            mesh.uv.push_back( aimesh->mTextureCoords[0][i].y );
        }
    }

    // Index
    for( uint i = 0; i < aimesh->mNumFaces; ++i ) {
        // Assume all faces are triangulate
        mesh.index.push_back( aimesh->mFaces[i].mIndices[0] );
        mesh.index.push_back( aimesh->mFaces[i].mIndices[1] );
        mesh.index.push_back( aimesh->mFaces[i].mIndices[2] );
    }

    // Skip joints if no matching armature found
    if(aimesh->mNumBones == 0  || !armatureMap.contains(mesh.name))
        return;

    ArmatureData &armature = armatureMap[mesh.name];
    // Fill all vertices with empty weights
    mesh.weightCount = 4;
    mesh.jointCount = 4;
    mesh.weight.resize(mesh.weightCount * mesh.vertexCount);
    mesh.joint.resize(mesh.jointCount * mesh.vertexCount);
    std::fill(mesh.weight.begin(), mesh.weight.end(), 0);
    std::fill(mesh.joint.begin(), mesh.joint.end(), 0);

    // Temporary vector to store the joints per-vertex
    vector<uint8_t> joints_added;
    joints_added.resize(mesh.vertexCount);
    std::fill(joints_added.begin(),joints_added.end(),0);

    aiBone *bone;
    aiVertexWeight *vweight;
    string joint_name;
    uint8_t joint_id = 0;
    for(uint i = 0; i < aimesh->mNumBones; ++i){
        bone = aimesh->mBones[i];
        joint_name = std::string_view(bone->mName.C_Str());
        if(!armature.joint_names.contains(joint_name))
            continue;
        joint_id = armature.joint_names[joint_name];
        for(uint j=0; j < bone->mNumWeights; ++j){
            // Insert the weight at the given vertex
            vweight = &bone->mWeights[j];
            vec4 &wvec = (vec4&)mesh.weight[vweight->mVertexId*4];
            wvec[joints_added[vweight->mVertexId]] = vweight->mWeight;

            // Insert the joint at the given vertex
            mesh.joint[vweight->mVertexId*4+joints_added[vweight->mVertexId]] = joint_id;
            ++joints_added[vweight->mVertexId];
        }
    }

}

void readHierarchy( aiNode *root ) {
    std::queue<aiNode *> nodequeue;
    aiNode *node;
    nodequeue.push( root );
    string name;

    while( !nodequeue.empty() ) {
        node = nodequeue.front();
        // If the node does not contain a mesh, add its children
        if( node->mNumMeshes == 0 ) {
            name = std::string_view( node->mName.C_Str() );

            // Read nodes explicitly marked as armature by the user
            if( name.starts_with("Armature_")) {
                readJointHierarchy(node);
            }

            // Ignore non-armature nodes and treat them as intermediates
            else {
                for( int i = 0; i < node->mNumChildren; ++i ) {
                    nodequeue.push( node->mChildren[i] );
                }
            }
        }
        else{
            // for(uint i=0;i<node->mNumMeshes;++i){
            //     aiMat4x4tomat4(node->mTransformation, meshMap[(string)std::string_view(node->mName.C_Str())].transform);
            // }
        }
        nodequeue.pop();
    }
}

void readJointHierarchy(aiNode *root){
    vector<aiNode *> nodes;
    string name, armature_name;
    armature_name = std::string_view( root->mName.C_Str() );
    armature_name.erase(0,9); // Remove the "Armature_" prefix

    // Create a map value to store the read hierarchy
    ArmatureData &armature = armatureMap[armature_name];
    armature.name = armature_name;


    // Create a list of nodes as well as the map of joint names
    unsigned int node_index = 0, joint_index = 0;
    string jointName;
    Joint joint;
    nodes.push_back(root);

    while( node_index < nodes.size()){
        jointName = std::string_view(nodes[node_index]->mName.C_Str());
        // Skip nodes with meshes
        if(nodes[node_index]->mNumMeshes > 0){
            aiMat4x4tomat4(root->mTransformation, meshMap[jointName].transform);
            glm_mat4_inv(meshMap[jointName].transform,meshMap[jointName].transform);
            ++node_index;
            continue;
        }

        // Place the joint name in the map
        armature.joint_names[jointName] = joint_index;

        // Create a new joint with the aiNode values
        joint = Joint();
        joint.name = jointName;
        aiMat4x4tomat4(nodes[node_index]->mTransformation, joint.local);
        armature.joints.push_back(joint);

        // Place all children on the back of the joint list and continue
        for(uint8_t i = 0; i < nodes[node_index]->mNumChildren; ++i ){
            nodes.push_back(nodes[node_index]->mChildren[i]);
        }
        ++node_index;
        ++joint_index;
    }

    // Link each joint to its parent, add the children when linking to the parent
    string parentName;
    uint8_t parentID;

    // Remove root transform and skip it when linking (will still have children)
    glm_mat4_identity(armature.joints[0].rest);
    glm_mat4_identity(armature.joints[0].local);
    node_index = joint_index = 0;

    while(joint_index < armature.joints.size()){
        if(nodes[node_index]->mNumMeshes > 0){
            ++node_index;
            continue;
        }
        parentName = std::string_view(nodes[node_index]->mParent->mName.C_Str());
        // WARNING does not check mapping, possibly switch to try/catch
        parentID = armature.joint_names[parentName];

        // Set parent, add as a child to parent
        armature.joints[joint_index].parent = parentID;
        armature.joints[parentID].children.push_back(joint_index);

        // Set rest transform to parent's rest transform with local transform
        glm_mat4_mul(
            armature.joints[parentID].rest,
            armature.joints[joint_index].local,
            armature.joints[joint_index].rest
        );
        ++joint_index;
        ++node_index;
    }

    printf("Joint Count: %d\n",(int)armature.joints.size());

    // armature.printData();
}

void readAnimationData(aiAnimation* ai_animation ){
    static std::regex valid_name("[a-zA-Z0-9_]+\\|[a-zA-Z0-9_]+");
    string anim_name;
    string armature_name;
    anim_name = std::string_view( ai_animation->mName.C_Str());
    if(!std::regex_match(anim_name,valid_name)){
        std::cout << "Invalid animation name: \"" << anim_name << "\". Must match <Armature Name>|<Action Name>" <<std::endl;
        return;
    }
    std::cout << "Animation: " << anim_name << std::endl;
    // Extract and delete the armature name prefix
    armature_name = anim_name.substr(0,anim_name.find("|"));
    anim_name.erase(0,armature_name.length()+1);

    if(!armatureMap.contains(armature_name)){
        std::cout << "No armature found with name: " << armature_name << std::endl;
        return;
    }

    ArmatureData &armature = armatureMap[armature_name];
    armature.animations.push_back(AnimationData());
    AnimationData &animation = armature.animations.back();
    animation.name = anim_name;
    animation.fromAiAnimation(armature, ai_animation);
}


void exportMeshData( MeshData *mesh ) {
    string filepath = "../dest_asset/" + mesh->name + ".ply";
    std::ofstream filewriter;
    filewriter.open( filepath, std::ios::out );
    if( !filewriter ) {
        std::cout << "Failed to write " << filepath << std::endl;
        return;
    }

    // Write the ply header ORDER MATTERS (the importer is not great)
    filewriter << "ply" << std::endl;
    filewriter << "format binary_little_endian 1.0" << std::endl;
    filewriter << "element vertex " << mesh->vertexCount << std::endl;
    if( mesh->posCount == 3 ) {
        filewriter << "property float x" << std::endl;
        filewriter << "property float y" << std::endl;
        filewriter << "property float z" << std::endl;
    }
    if( mesh->normCount == 3 ) {
        filewriter << "property float nx" << std::endl;
        filewriter << "property float ny" << std::endl;
        filewriter << "property float nz" << std::endl;
    }
    if( mesh->uvCount == 2 ) {
        filewriter << "property float s" << std::endl;
        filewriter << "property float t" << std::endl;
    }
    if( mesh->colCount == 3 ) {
        filewriter << "property uchar red" << std::endl;
        filewriter << "property uchar green" << std::endl;
        filewriter << "property uchar blue" << std::endl;
    }
    if( mesh->weightCount == 4 ) {
        filewriter << "property float w1" << std::endl;
        filewriter << "property float w2" << std::endl;
        filewriter << "property float w3" << std::endl;
        filewriter << "property float w4" << std::endl;
    }
    if( mesh->jointCount == 4 ) {
        filewriter << "property uchar j1" << std::endl;
        filewriter << "property uchar j2" << std::endl;
        filewriter << "property uchar j3" << std::endl;
        filewriter << "property uchar j4" << std::endl;
    }

    filewriter << "element face " << mesh->faceCount << std::endl;
    filewriter << "property list uchar uint vertex_indices" << std::endl;
    filewriter << "end_header" << std::endl;

    // Switch to Binary mode and append raw data
    std::fstream::pos_type writepos = filewriter.tellp();
    filewriter.clear();
    filewriter.close();
    filewriter.open( filepath, std::ios::binary | std::ios::app );
    filewriter.seekp( writepos );

    // Buffers to store data before being sent
    char buffer[16];
    float *floatbuffer = reinterpret_cast<float *>( buffer );

    // Write the vertex data section
    for( uint32_t i = 0; i < mesh->vertexCount; i++ ) {
        if( mesh->posCount == 3 ) {
            memcpy(&buffer[0], &mesh->pos[i*3],12);
            filewriter.write( buffer, 12 );
        }
        if( mesh->normCount == 3 ) {
            memcpy(&buffer[0], &mesh->norm[i*3],12);
            filewriter.write( buffer, 12 );
        }
        if( mesh->uvCount == 2 ) {
            memcpy(&buffer[0], &mesh->uv[i*2],8);
            filewriter.write( buffer, 8 );
        }
        if( mesh->colCount == 3 ) {
            buffer[0] = uint8_t( mesh->col[i * 3] * UINT8_MAX );
            buffer[1] = uint8_t( mesh->col[i * 3 + 1] * UINT8_MAX );
            buffer[2] = uint8_t( mesh->col[i * 3 + 2] * UINT8_MAX );
            filewriter.write( buffer, 3 );
        }
        if( mesh->weightCount == 4 ) {
            memcpy(&buffer[0], &mesh->weight[i*4],16);
            filewriter.write( buffer, 16 );
        }
        if( mesh->jointCount == 4 ) {
            memcpy(&buffer[0], &mesh->joint[i*4],4);
            filewriter.write( buffer, 4 );
        }
    }

    // Write face data, 1st byte is face count, next 12 bytes are the indices
    for(uint32_t i = 0; i < mesh->faceCount; i++){
        buffer[0] = 3;
        memcpy(&buffer[1], &mesh->index[i*3],12);
        filewriter.write( buffer, 13 );
    }

    filewriter.close();
}

void exportArmatureData( ArmatureData *armature ) {
    string filepath = "../dest_asset/" + armature->name + ".arm";
    std::ofstream filewriter;
    filewriter.open( filepath, std::ios::out | std::ios::binary );
    if( !filewriter ) {
        std::cout << "Failed to write " << filepath << std::endl;
        return;
    }

    uint8_t joint_count = (uint8_t)armature->joints.size();

    // Write number of joints, maximum of 255
    filewriter.write(reinterpret_cast<char*>(&joint_count),1);

    // Write each joint
    Joint *joint;
    vec3 pos;
    versor rot;
    uint8_t child_count, name_length, channel_count;
    uint32_t key_count;
    for(uint8_t i=0; i < joint_count; ++i){
        joint = &armature->joints[i];

        // Name
        name_length = (uint8_t)joint->name.length();
        filewriter.write(reinterpret_cast<char*>(&name_length),1);
        filewriter << joint->name;

        // Parent
        filewriter.write(reinterpret_cast<char*>(&joint->parent),1);

        // Children count
        child_count = (uint8_t)joint->children.size();
        filewriter.write(reinterpret_cast<char*>(&child_count),1);

        // Children
        for(uint8_t j = 0; j < child_count; ++j ){
            filewriter.write(reinterpret_cast<char*>(&joint->children[j]),1);
        }

        // Local Transform position and rotation
        glm_mat4_quat( joint->local, rot );
        glm_vec3_copy( joint->local[3], pos );
        filewriter.write(reinterpret_cast<char*>(pos),12);
        filewriter.write(reinterpret_cast<char*>(rot),16);

    }


    // Write Animations
    uint8_t animation_count = (uint8_t)armature->animations.size();
    filewriter.write(reinterpret_cast<char*>(&animation_count),1);
    for(AnimationData &anim : armature->animations){
        // Name
        name_length = (uint8_t)anim.name.length();
        filewriter.write(reinterpret_cast<char*>(&name_length),1);
        filewriter << anim.name;

        // Duration
        filewriter.write(reinterpret_cast<char*>(&anim.duration), 4);

        // Number of channels
        channel_count = (uint8_t)anim.channels.size();
        filewriter.write(reinterpret_cast<char*>(&channel_count),1);

        // For each channel
        for(Channel &channel : anim.channels){
            // Joint ID
            filewriter.write(reinterpret_cast<char*>(&channel.joint),1);

            // Number of keys
            key_count = channel.position.size();
            filewriter.write(reinterpret_cast<char*>(&key_count),4);
            key_count = channel.rotation.size();
            filewriter.write(reinterpret_cast<char*>(&key_count),4);

            // Position keys
            for(poskey &key:channel.position){
                filewriter.write(reinterpret_cast<char*>(&key.t),4);
                filewriter.write(reinterpret_cast<char*>(&key.val),12);
            }

            // Rotation keys
            for(rotkey &key:channel.rotation){
                filewriter.write(reinterpret_cast<char*>(&key.t),4);
                filewriter.write(reinterpret_cast<char*>(&key.val),16);
            }
        }
    }

    filewriter.close();
}

