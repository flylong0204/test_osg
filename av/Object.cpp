#include "stdafx.h"
#include "creators.h"
#include "LOD.h"

namespace {

struct fpl_wrap 
{
	fpl_wrap(const std::string& name)
	{
		fpl_.push_back(cfg().path.data + "/models/" + name + "/");
		fpl_.push_back(cfg().path.data + "/areas/" + name + "/");
	};

	osgDB::FilePathList fpl_;
};

}

namespace creators
{

typedef std::map< std::string, osg::ref_ptr<osg::Node> > nodesMap;  

nodesMap objCache;

osg::Node* createObject(std::string name, bool fclone)
{
	fpl_wrap fpl(name);
	osg::Node* object_file = nullptr;
	nodesMap::iterator it;
    


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


        MaterialVisitor::namesList nl;
        nl.push_back("building");
        nl.push_back("default");
        nl.push_back("plane");

        
        MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,name,sp::_2,sp::_3),creators::computeAttributes,mat::reader::read(mat_file_name));
        object_file->accept(mv);

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

		auto tail = addAsChild("tail",white_light);
		auto strobe_r = addAsChild("strobe_r",white_light);
		auto strobe_l = addAsChild("strobe_l",white_light);

		auto port = addAsChild("port",green_light);
		auto star_board = addAsChild("starboard",red_light);
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

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();   

        auto pat = object_file->asTransform()->asPositionAttitudeTransform();
        pat->setName("pat");
        pat->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
        pat->setPosition(osg::Vec3(0,airplane?-(ym)/2.0f:0.f,0)); // FIXME �������� �������� � �� ������� ���� � ��� ������

		objCache[name] = object_file;

	}

	return object_file;
}

}