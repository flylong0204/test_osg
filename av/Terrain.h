#pragma once

#include "Lights.h"

 namespace avTerrain
 {
     class Terrain : public osg::Group
     {
     public:
         Terrain (osg::Group* sceneRoot);
         /*osg::Node* */ 
         void create( std::string name );
     protected:
         void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset);
     private:
         void cull( osg::NodeVisitor * pNV );
     private:
         osg::observer_ptr<osg::Group>   _sceneRoot;
         // Dynamic lights handler
         avScene::LightNodeHandler      _lightsHandler;
           
     };

 }