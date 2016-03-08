#include "stdafx.h"
#include "av/precompiled.h"

#include "av/avScene/Scene.h"
#include "async_load.h"
#include "utils/callbacks.h"

namespace Utils
{
    LoadManager::LoadManager()
    {
        setUpdateCallback(Utils::makeNodeCallback(this, &LoadManager::update));
    }

    // Use a thread to call osgDB::readNodeFile.
    void LoadManager::update( osg::NodeVisitor * nv )
    {
        avScene::Scene * scene = avScene::GetScene();

        if (scene == NULL)
            return;

        for (auto it = threads_.begin(); it!= threads_.end();++it)
        {
            if(!(*it)->isRunning())
            {
               if((*it)->_node.valid())
               {
				   scene->getTerrainRoot()->addChild((*it)->_node);
				   (*it)->_sig();
			   }
               delete *it;
               threads_.erase(it);
               break;
            }
        }

        
        
    }

    void LoadManager::load ( osg::MatrixTransform* mt, LoadNodeThread::worker_f work , LoadNodeThread::set_signal_f s)
    {
        threads_.push_back(new LoadNodeThread(work,s));
    }

}