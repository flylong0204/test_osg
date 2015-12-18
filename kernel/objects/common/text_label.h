#pragma once 


namespace visual_objects
{
    struct label_support
    {
        label_support(visual_object_ptr obj,const std::string& text )
            : visual_object_(obj)
            , label_            (nullptr) 
        {
            init_(text);
        }

        inline void update( cg::polar_point_3f const  &dir, point_3f offset )
        {
            update_(dir,offset); 
        }

    private:

        inline void init_(const std::string&  text)
        {
            label_ = findFirstNode((visual_object_)->root(),"text_label");

            osg::ComputeBoundsVisitor cbvs;
            label_->accept( cbvs );
            const osg::BoundingBox bb_s = cbvs.getBoundingBox();

            const auto& l = label_->asGeode()->getDrawableList();

            for(auto itr=l.begin();
                itr!= l.end();
                ++itr)
            {
                if(auto dtext =  dynamic_cast<osgText::Text*>(itr->get()))
                {
                    dtext->setText(text);
                }
            }
        }


        inline void update_(cg::polar_point_3f const  &dir, point_3f offset )
        {
        }

        double                radius_;
        osg::Node *           label_;

        visual_object_ptr     visual_object_;
    };

}