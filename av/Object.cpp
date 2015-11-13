#include "stdafx.h"
#include "Object.h"
#include "avCore/LOD.h"
#include "avLights/Lights.h"
#include "avLights/LightManager.h"
#include "avCore/Utils.h"

#include "visitors/materials_visitor.h"
#include "materials.h"

namespace creators
{

	struct fpl_wrap 
	{
		fpl_wrap(const std::string& name)
		{
			fpl_.push_back(cfg().path.data + "/models/" + name + "/");
			fpl_.push_back(cfg().path.data + "/areas/" + name + "/");
		};

		osgDB::FilePathList fpl_;
	};



typedef std::map< std::string, osg::ref_ptr<osg::Node> > nodesMap;  

nodesMap objCache;

void  releaseObjectCache()
{
        objCache.clear();
}

osg::Node* createObject(std::string name, bool fclone)
{
	fpl_wrap fpl(name);
	osg::Node* object_file = nullptr;
	nodesMap::iterator it;
    
    osg::PositionAttitudeTransform* pat;

	if(( it = objCache.find(name))!=objCache.end())
	{
		if(fclone)
		object_file = osg::clone(it->second.get(), osg::CopyOp::DEEP_COPY_ALL 
			& ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
			& ~osg::CopyOp::DEEP_COPY_ARRAYS
			& ~osg::CopyOp::DEEP_COPY_IMAGES
			& ~osg::CopyOp::DEEP_COPY_TEXTURES
			& ~osg::CopyOp::DEEP_COPY_STATESETS  
			& ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
			& ~osg::CopyOp::DEEP_COPY_UNIFORMS
			& ~osg::CopyOp::DEEP_COPY_DRAWABLES
			);
		else
			object_file = it->second.get();

        pat = object_file->asTransform()->asPositionAttitudeTransform();
	}
	else
	{
		std::string object_file_name =  osgDB::findFileInPath(name + ".osgb", fpl.fpl_,osgDB::CASE_INSENSITIVE);
        std::string mat_file_name = osgDB::findFileInPath(name+".dae.mat.xml", fpl.fpl_,osgDB::CASE_INSENSITIVE);
        

		if(object_file_name.empty())
			object_file_name = osgDB::findFileInPath(name+".dae", fpl.fpl_,osgDB::CASE_INSENSITIVE);

		if(object_file_name.empty())
			return nullptr;

		object_file = osgDB::readNodeFile(object_file_name);

        bool airplane = findFirstNode(object_file ,"shassi_",findNodeVisitor::not_exact)!=nullptr;
        bool vehicle  = findFirstNode(object_file ,"wheel",findNodeVisitor::not_exact)!=nullptr;
        FIXME(����� ��� � ������� ��� ����������� ������)
        bool heli     = findFirstNode(object_file ,"tailrotor",findNodeVisitor::not_exact)!=nullptr;


		osg::Node* engine = nullptr; 
		osg::Node* lod0 =  findFirstNode(object_file,"Lod0"); 
		osg::Node* lod3 =  findFirstNode(object_file,"Lod3"); 

		osg::Group* root =  findFirstNode(object_file,"Root")->asGroup(); 

        root->setUserValue("id",reinterpret_cast<uint32_t>(&*root));
		// object_file->setName(name);
	
		// � ����� engine �� ������?
		//engine =  findFirstNode(object_file,"engine",findNodeVisitor::not_exact);
		//if (engine) engine_geode = engine->asGroup()->getChild(0);

		// FIXME �� � ���� � ��������� ����������
#if 0
#if 0
		auto CreateLight = [=](const osg::Vec4& fcolor,const std::string& name,osg::NodeCallback* callback)->osg::Geode* {
			osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable();
			shape1->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.2f) );
			osg::Geode* light = new osg::Geode;
			light->addDrawable( shape1.get() );
			dynamic_cast<osg::ShapeDrawable *>(light->getDrawable(0))->setColor( fcolor );
			light->setUpdateCallback(callback);
			light->setName(name);
			const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE| osg::StateAttribute::OFF;
			light->getOrCreateStateSet()->setAttribute(new osg::Program(),value);
			light->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
			light->getOrCreateStateSet()->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
			light->getOrCreateStateSet()->setMode( GL_LIGHTING, value );
			return light;
		};

		osg::ref_ptr<osg::Geode> red_light   = CreateLight(red_color,std::string("red"),nullptr);
		osg::ref_ptr<osg::Geode> blue_light  = CreateLight(blue_color,std::string("blue"),nullptr);
		osg::ref_ptr<osg::Geode> green_light = CreateLight(green_color,std::string("green"),nullptr);
		osg::ref_ptr<osg::Geode> white_light = CreateLight(white_color,std::string("white_blink"),new effects::BlinkNode(white_color,gray_color));

		auto addAsChild = [=](std::string root,osg::Node* child)->osg::Node* {
			auto g_point =  findFirstNode(object_file,root.c_str());
			if(g_point)  
			{
				g_point->asGroup()->addChild(child);
			}
			return g_point;
		};

		auto tail       = addAsChild("tail",white_light);
		auto strobe_r   = addAsChild("strobe_r",white_light);
		auto strobe_l   = addAsChild("strobe_l",white_light);

		auto port       = addAsChild("port",green_light);
		auto star_board = addAsChild("starboard",red_light);
#else  
        findNodeVisitor::nodeNamesList list_name;

        osgSim::LightPointNode* obj_light =  new osgSim::LightPointNode;

        const char* names[] =
        {
            "port",
            "starboard",
            "tail",
            "steering_lamp",
            "strobe_",
            "landing_lamp",
            "back_tail",
            // "navaid_",
        };

        for(int i=0; i<sizeof(names)/sizeof(names[0]);++i)
        {
            list_name.push_back(names[i]);
        }

        findNodeVisitor findNodes(list_name,findNodeVisitor::not_exact); 
        root->accept(findNodes);
        
        findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();
        
        auto shift_phase = cg::rand(cg::range_2(0, 255));
        
        osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));
        
        for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
        {
             osgSim::LightPoint pnt;
             bool need_to_add = false;

             if((*it)->getName() == "tail")
             { 
                 pnt._color      = white_color;
                 need_to_add     = true;
             }

             if((*it)->getName() == "port")
             {   
                 pnt._color      = green_color;
                 need_to_add     = true;
                 pnt._sector = sector;
             }

             if((*it)->getName() == "starboard") 
             {
                 pnt._color = red_color;
                 need_to_add     = true;
                 pnt._sector = sector;
             }
            
             
             if(boost::starts_with((*it)->getName(), "strobe_")) 
             {
                 pnt._color  = white_color;
                 pnt._blinkSequence = new osgSim::BlinkSequence;
                 pnt._blinkSequence->addPulse( 0.05,
                     osg::Vec4( 1., 1., 1., 1. ) );

                 pnt._blinkSequence->addPulse( 1.5,
                     osg::Vec4( 0., 0., 0., 0. ) );

                 pnt._sector = new osgSim::AzimSector(-osg::inDegrees(170.0),-osg::inDegrees(10.0),osg::inDegrees(90.0));

                 pnt._blinkSequence->setPhaseShift(shift_phase);
                 need_to_add     = true;
             }

             pnt._position = (*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans();
             pnt._radius = 0.2f;
             if(need_to_add)
                 obj_light->addLightPoint(pnt);
        }

        if(wln_list.size()>0)
            object_file->asGroup()->addChild(obj_light);


#endif
#endif
        //
		//  � ����� ����� ������ ���������� ���
		//
        osg::Node* lod_ =  findFirstNode(object_file,"lod_",findNodeVisitor::not_exact);

        if(lod_)  // ������ �� ��� ������ ����� �������� ��������� ��
        {
 		    avLod::LOD* lod = new avLod::LOD;

		    lod_->asGroup()->addChild(lod);
		    lod->addChild(lod0,0,1200);
		    lod->addChild(lod3,1200,50000);
        }
        else
        {
#if 0
            osg::Node* named_node =  findFirstNode(object_file,name,findNodeVisitor::not_exact);

            lod3 = osg::clone(named_node, osg::CopyOp::DEEP_COPY_ALL 
                & ~osg::CopyOp::DEEP_COPY_IMAGES
                & ~osg::CopyOp::DEEP_COPY_TEXTURES
                & ~osg::CopyOp::DEEP_COPY_STATESETS  
                & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
                & ~osg::CopyOp::DEEP_COPY_UNIFORMS
                );


            osgUtil::Simplifier simplifier;
            simplifier.setSampleRatio( 0.001 );
            lod3->accept( simplifier );
            lod3->setName("Lod3");
            
            root->addChild(lod3);
#endif     
        }

        osg::ComputeBoundsVisitor cbv;
        object_file->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();
        /*
        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();*/   
        
        float xm = abs(bb.xMax()) + abs(bb.xMin());
        float ym = abs(bb.yMax()) + abs(bb.yMin());
        float zm = abs(bb.zMax()) + abs(bb.zMin());
        
        //float dx = abs(bb.xMax()) - xm / 2.f;
        //float dy = abs(bb.yMax()) - ym / 2.f;
        //float dz = abs(bb.zMax()) - zm / 2.f;
        
        float dx = -xm / 4.f; 
        float dy = -ym / 4.f; 
        float dz = -zm / 4.f; 
        
        if (object_file->asTransform())
        {
            pat = object_file->asTransform()->asPositionAttitudeTransform();
            pat->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
            pat->setPosition(osg::Vec3(0,airplane?dz:0.f,0)); // FIXME �������� �������� � �� ������� ���� � ��� ������
            
        }
        else
        {
            pat = new osg::PositionAttitudeTransform; 
            pat->addChild(object_file);
            pat->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
            pat->setPosition(osg::Vec3(0.,airplane?dy:0.f,0.)); // FIXME �������� �������� � �� ������� ���� � ��� ������

        }
        
        MaterialVisitor::namesList nl;
        nl.push_back("building");
        nl.push_back("default");
        nl.push_back("plane");
        //nl.push_back("rotor"); /// �������������� �������������� � ������������ �������
        
        MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,sp::_2,name,sp::_3,sp::_4),/*nullptr*//*[=](osg::Node* model,std::string mat_name){}*/creators::computeAttributes,mat::reader::read(mat_file_name));
        pat->accept(mv);



        pat->setName("pat");
        
#if 1
        if(name=="towbar")
        {
           auto towpoint =  findFirstNode(object_file,"tow_point"); 

           pat->setPosition(osg::Vec3(0,zm/2,towpoint->asTransform()?-towpoint->asTransform()->asMatrixTransform()->getMatrix().getTrans().z():0));
           
        }
#endif

		objCache[name] = pat/*object_file*/;

	}

    pat->setNodeMask( PICK_NODE_MASK | REFLECTION_MASK );

	return pat/*object_file*/;
}

}
