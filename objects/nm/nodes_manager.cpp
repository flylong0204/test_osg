#include "stdafx.h"

#include "precompiled_objects.h"

#include "objects/nodes_management.h"
#include "nodes_manager.h"
#include "node_impl.h"

namespace nodes_management
{   
    class manager_impl;

    // FIXME ���� ����� ����
    void /*system_base::*/block_obj_msgs(bool block)
    {

    }

    void /*system_base::*/send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd)
    {

    } 

    manager_ptr create_manager(osg::Node* node) 
    {
        size_t id  = 0x666;
        auto msg_service = boost::bind(&send_obj_message, id, _1, _2, _3);
        auto block_msgs  = [=](bool block){ block_obj_msgs(block); };
        kernel::object_create_t  oc(
            nullptr, 
            nullptr,                    // kernel::system*                 sys             , 
            id,                         // size_t                          object_id       , 
            "name",                     // string const&                   name            , 
            std::vector<object_info_ptr>(),  // vector<object_info_ptr> const&  objects         , 
            msg_service,                // kernel::send_msg_f const&       send_msg        , 
            block_msgs                  // kernel::block_obj_msgs_f        block_msgs
            );

        return boost::make_shared<manager_impl>(node,oc);
    }

    class manager_impl 
        :  public base_view_presentation
        ,  public manager
    {
    public: 
        manager_impl( osg::Node* base, kernel::object_create_t const& oc) 
            : base_view_presentation(oc)
            , base_(base)
            
        {}

        object_info_ptr manager_impl::create(osg::Node* base,kernel::object_create_t const& oc/*, dict_copt dict*/);
        void init();

        node_info_ptr find_node(std::string const& name) const override;
        node_info_ptr get_node    (uint32_t node_id)   const  override;
        void          set_model   (string const& model, bool save_root_pos = true) override;
        string const&   get_model   () const override;
   
    private: 
        osg::ref_ptr<osg::Node> base_;
        std::string       model_name_;
    };

    node_info_ptr manager_impl::find_node(std::string const& name) const
    {
        //for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        //    if ((*it)->data().name == name)
        //        return (*it);
        auto n = findFirstNode(base_,name,findNodeVisitor::not_exact);
        if(n)
            return boost::make_shared<node_impl>(n);

        return node_info_ptr();
    }

    node_info_ptr   manager_impl::get_node    (uint32_t node_id)   const
    {
          
          if(node_id==0)
                  return boost::make_shared<node_impl>(base_);   // FIXME �� ���� ��

          return node_info_ptr();
    }

    void  manager_impl::set_model   (string const& model, bool save_root_pos) 
    {
        // FIXME TODO � �������� ��������?   
        model_name_ =  model;
    }

    string const&   manager_impl::get_model   () const 
    {
          return model_name_;
    }

    object_info_ptr manager_impl::create(osg::Node* base,kernel::object_create_t const& oc/*, dict_copt dict*/)
    {
        manager_impl *obj = new manager_impl(base,oc/*, dict*/);
        object_info_ptr info(obj);
        obj->init();
        return info;
    }

    void /*view*/manager_impl::init()
    {
        //init_collision_volume();

        //if (contains_model_)
        //{
        //    binary::input_stream is(model_data_);
        //    binary::size_type count;

        //    read(is, count);
        //    for (binary::size_type i = 0; i < count; ++i)
        //    {
        //        auto nod = create_node(this, is);
        //        nodes_.insert(nod->node_id(), nod);
        //    }
        //}
        //else
        //{
        //    init_logic_tree();
        //    contains_model_ = true;
        //}
    }
}
