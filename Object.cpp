#include "stdafx.h"
#include "find_node_visitor.h"
#include "creators.h"
#include "LOD.h"

namespace {

struct fpl_wrap 
{
	fpl_wrap(const std::string& name)
	{
		fpl_.push_back("planes/" + name + "/");
		fpl_.push_back("vehicles/" + name + "/");
		fpl_.push_back("areas/" + name + "/");
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

		if(object_file_name.empty())
			object_file_name = osgDB::findFileInPath(name+".dae", fpl.fpl_,osgDB::CASE_INSENSITIVE);

		if(object_file_name.empty())
			return nullptr;

		object_file = osgDB::readNodeFile(object_file_name);

		avLod::LOD* lod = new avLod::LOD;

		osg::Node* engine = nullptr; 
		osg::Node* lod0 =  findFirstNode(object_file,"Lod0"); 
		osg::Node* lod3 =  findFirstNode(object_file,"Lod3"); 

		osg::Group* root =  findFirstNode(object_file,"Root")->asGroup(); 
		osg::Group* lod_ =  findFirstNode(object_file,"lod_",findNodeVisitor::not_exact)->asGroup();

		object_file->setName(name);
	
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

		lod_->addChild(lod);
		lod->addChild(lod0,0,1200);
		lod->addChild(lod3,1200,50000);
	}

	return object_file;
}

}