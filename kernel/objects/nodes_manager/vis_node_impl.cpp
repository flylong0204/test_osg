#include "vis_node_impl.h"
#include "nodes_manager_view.h"
#include "nodes_manager_visual.h"

#include "av/avCore/Global.h"

namespace nodes_management
{

vis_node_impl::vis_node_impl( view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id )
    : node_impl(manager, parent, data, id)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

vis_node_impl::vis_node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id )
    : node_impl(manager, pos, data, id)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

vis_node_impl::vis_node_impl( view * manager, binary::input_stream & stream )
    : node_impl(manager, stream)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

void vis_node_impl::on_visual_object_created()
{
    fill_visual_nodes();
	need_update_ = true;
    sync_position();

	for (auto it=anim_queue_.begin(); it!=anim_queue_.end();++it )
	{
		on_animation(*it, true);
	}

	anim_queue_.clear();
}

void vis_node_impl::fill_visual_nodes()
{
    visual* vis_manager = dynamic_cast<visual*>(manager_);
    visual_nodes_.clear();
    auto vo = vis_manager->visual_object();

    if (vo)
    {
        if(data_.node_ids.size() == 0)
        for (auto jt = data_.victory_nodes.begin(); jt != data_.victory_nodes.end(); ++jt)
        {
            if (*jt=="root")
            {
                 visual_nodes_.push_back(vo->node().get());
                 continue;
            }

            // if (auto visnode = findFirstNode(vo->node().get(), *jt))
            if (auto visnode = vo->get_node(*jt))
            {
                visual_nodes_.push_back(visnode);

                FIXME(fill_visual_nodes Need to be realized)
                //if (texture_ && visnode->as_root())
                //    visnode->as_root()->set_base_texture(*texture_);
            }
        }
        else
        for (auto jt = data_.node_ids.begin(); jt != data_.node_ids.end(); ++jt)
        {
            if (*jt=="root")
            {
                visual_nodes_.push_back(vo->node().get());
                continue;
            }

            //if (auto visnode = findFirstNode(vo->node().get(), *jt,
            //    findNodeVisitor::exact, osg::NodeVisitor::TRAVERSE_ALL_CHILDREN, findNodeVisitor::user_id
            //    ))

            if (auto visnode = vo->get_node(*jt))
            {
                visual_nodes_.push_back(visnode);

                FIXME(fill_visual_nodes Need to be realized)
                    //if (texture_ && visnode->as_root())
                    //    visnode->as_root()->set_base_texture(*texture_);
            }
        }
    }

}

void vis_node_impl::extrapolated_position_reseted()
{
    visible_.reset();
    need_update_ = true;
}

void vis_node_impl::pre_update(double time)
{
    node_impl::pre_update(time);
    
	if (!time_)
        return;

    if(!(extrapolated_position_.is_local()))
    {    
        //force_log fl; 

        double dt = time - *time_;
        
        LOG_ODS_MSG( "vis_node_impl::pre_update(double time) [ " << name() << " ]:    dt=" << dt 
            << "   time: " << time
            << "   time_: " << *time_
            << "\n" );

    }

    sync_position();

	last_update_ = time;
}

void vis_node_impl::on_animation(msg::node_animation const& anim, bool deffered)
{
    LOG_ODS_MSG("vis_node_impl::on_animation: " << anim.name << "\n" );

    nm::visit_sub_tree(manager_->get_node_tree_iterator(node_id()), [this](nm::node_info_ptr n)->bool
    {
        dynamic_cast<vis_node_impl *>(n.get())->sync_position();
        return true;
    });

    FIXME(Animation Need to be realized)
    //for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    //{
    //    if (auto v_node = (*(it))->as_animgroup())
    //        v_node->play_sequence(victory::animation_player::sequence_play_info(anim.name, anim.len, float(anim.time), anim.from, anim.size));
    //}

    // FIXME  �������� ��� ��������
    FIXME(����� ��� ����� �������)
    
    visual* vis_manager = dynamic_cast<visual*>(manager_);

    auto manager_ =  vis_manager->visual_object()->animation_manager();

    bool bopen = (cg::eq(anim.from,0.f)) && (cg::eq(anim.size,0.f)) || (anim.from >= 1.0) && (anim.size < 0);
    osgAnimation::Animation::PlayMode pm = bopen?osgAnimation::Animation::ONCE_BACKWARDS:osgAnimation::Animation::ONCE;
    
    pm = anim.from < 0?osgAnimation::Animation::LOOP:pm;

#if 1
    if (visual_nodes_.size()>0)
    {

    auto root = visual_nodes_[0];
#if 0
    node_     = visual_nodes_[0];
#endif
    
        if ( manager_.valid() )
        {   
            const osgAnimation::AnimationList& animations =
                manager_->getAnimationList();

		    const bool f = anim.name == "Default";
		
		    const bool need_to_stop_now = current_anim != anim.name && cg::eq_zero(anim.cross_fade);
		
		    boost::optional<std::string> new_anim_name;
 
		    if(current_anim != anim.name && anim.cross_fade> 0.0 ) 
		      deferred_cross_fade = make_pair(*last_update_ + anim.cross_fade, anim.name);
		    else
		    for ( unsigned int i=0; i<animations.size(); ++i )
            {
                const std::string& name = animations[i]->getName();
                
                const bool is_playing = manager_->isPlaying(name);
		        
                if( (!is_playing ||  is_playing &&  animations[i]->getPlayMode() != pm ) &&(f? true : name == anim.name))
                {
                    if(anim.len >0) animations[i]->setDuration(anim.len);
                    animations[i]->setPlayMode(pm);
				
                    manager_->playAnimation( animations[i].get(),2,2.0 );
				    new_anim_name = name;
			    }
			
			    if(need_to_stop_now && current_anim == name)
				    manager_->stopAnimation(animations[i]);
            }

		    if(new_anim_name)
			    current_anim =  *new_anim_name;

        }

    }
	else
	{
        if(!deffered)
		    anim_queue_.push_back(anim);

        FIXME(�� ���� ����� ��� ����� ��� ������)
	}
#endif

}

void vis_node_impl::on_visibility(msg::visibility_msg const& m)
{	
    user_visible_ = m.visible;
    need_update_ = true;
}

void vis_node_impl::on_texture(msg::node_texture_msg const& m)
{
    node_impl::on_texture(m.tex);
    
    FIXME(on_texture Need to be realized)
    //for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    //{
    //    if (auto v_node = (*(it))->as_root())
    //    {
    //        v_node->set_base_texture(*texture_);
    //    }
    //}
}

bool vis_node_impl::is_visible() const
{
    if (!user_visible_)
        return false;

    double const visibility_distance = 30000;

    if (!visible_)
    {
        if (position_.is_local())
        {
            node_info_ptr rootnode = root_node();
            visible_ = rootnode ? vis_node_impl_ptr(rootnode)->is_visible() : false;
        }
        else
        {
            geo_point_3 base = dynamic_cast<visual_system_props*>(dynamic_cast<visual*>(manager_)->vis_system())->vis_props().base_point;
            visible_ = cg::distance(base, position_.global().pos) < visibility_distance;
        }
    }

    return *visible_;
}

void vis_node_impl::set_visibility  (bool visible)
{
    user_visible_ = visible;
	need_update_ = true;
}

void vis_node_impl::init_disp()
{                   
    msg_disp()
        .add<msg::node_animation>(boost::bind(&vis_node_impl::on_animation , this, _1, false))
        .add<msg::visibility_msg>(boost::bind(&vis_node_impl::on_visibility, this, _1));
}

void vis_node_impl::sync_position()
{
    if (!need_update_)
        return;

    visual* vis_manager = dynamic_cast<visual*>(manager_);

    if (!vis_manager->visual_object())
        return ;

    bool visible = is_visible();

    for (auto it = visual_nodes_.begin(); it != visual_nodes_.end(); ++it)
    {
        (*(it))->setNodeMask(visible?REFLECTION_MASK/*0x00010000*/:0);   // set_process_flag(visible);
    }

    if (visible)
    {
        if (position_.is_local())
        {
            cg::transform_4f tr(cg::as_translation(point_3f(extrapolated_position_.local().pos)), rotation_3f((extrapolated_position_.local().orien ).rotation()));
            for (auto it = visual_nodes_.begin(); it != visual_nodes_.end(); ++it)
                if ((*(it))->asTransform() && !boost::starts_with((*(it))->getName(),"shassi_") )
                if((*(it))->asTransform()->asMatrixTransform())
                    (*(it))->asTransform()->asMatrixTransform()->setMatrix(to_osg_transform(tr));
        }
        else
        {
            geo_base_3 base = dynamic_cast<visual_system_props*>(dynamic_cast<visual*>(manager_)->vis_system())->vis_props().base_point;
            point_3f offset = base(extrapolated_position_.global().pos);

            cg::transform_4f tr(cg::as_translation(offset), rotation_3f((extrapolated_position_.global().orien ).rotation()));
            for (auto it = visual_nodes_.begin(); it != visual_nodes_.end(); ++it)
                if ((*(it))->asTransform() && !boost::starts_with((*(it))->getName(),"shassi_") )
                if((*(it))->asTransform()->asMatrixTransform())
                (*(it))->asTransform()->asMatrixTransform()->setMatrix(to_osg_transform(tr));

            prev_extrapolated_position_  = extrapolated_position_;
        }
    }

    need_update_ = false;
}

vis_node_control::vis_nodes_t const& vis_node_impl::vis_nodes() const
{
    return visual_nodes_;
}
             
} // nodes_management
