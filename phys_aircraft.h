#pragma once



//#include "../common/phys_sys.h"
//#include "meteo/meteo.h"

using namespace cg;

#include "geometry/primitives.h"
#include "position.h"
#include "aircraft_common.h" 

namespace aircraft
{

    struct phys_aircraft_impl
        : phys_aircraft
    {
        static phys_aircraft_ptr create(cg::geo_base_3 const& base, phys::system_ptr phys_sys, 
 /*                                      meteo::meteo_cursor_ptr meteo_cursor, */
                                       //nodes_management::manager_ptr nodes_manager, 
                                       geo_position const& initial_position, 
                                       ada::data_t const& fsettings, 
                                       //shassis_support_ptr shassis, 
                                       size_t zone);

        phys_aircraft_impl(cg::geo_base_3 const& base, phys::system_ptr phys_sys, 
                           //meteo::meteo_cursor_ptr meteo_cursor, 
                           //nodes_management::manager_ptr nodes_manager, 
                           geo_position const& initial_position, 
                           ada::data_t const& fsettings, 
                           //shassis_support_ptr shassis, phys::compound_sensor_t const& s, 
                           size_t zone);

        ~phys_aircraft_impl();


    // phys_aircraft
    private:
        void update() override;
        //void attach_tow(bool attached) override;
        void go_to_pos(cg::geo_point_3 const& pos, quaternion const& orien) override;
        //geo_position get_position() const;
        //void set_air_cfg(fms::air_config_t cfg);
        //void set_prediction(double prediction);
        //geo_position get_wheel_position( size_t i ) const ;
        phys::rigid_body_ptr get_rigid_body() const;
        void set_steer   (double steer);
        //std::vector<phys::aircraft::contact_info_t> get_body_contacts() const;
        //bool has_wheel_contact(size_t id) const;
        //double wheel_skid_info(size_t id) const;
        //void remove_wheel(size_t id);
        //size_t get_zone() const;
        //void set_malfunction(bool malfunction);

    private:
        void create_phys_aircraft(geo_position const& initial_position, ada::data_t const& fsettings/*, phys::compound_sensor_t const& s*/);
        void sync_phys(double dt);
        void calc_phys_controls(double & slide_angle, double & thrust, double & attack_angle, double q, cg::rotation_3 const& vel_rotation, point_3 const& desired_accel, point_3 const& /*wind*/, bool reverse, bool low_attack);

    private:
        cg::geo_base_3                  base_;
        phys::system_ptr                phys_sys_;
        //nodes_management::manager_ptr nodes_manager_;
        //meteo::meteo_cursor_ptr       meteo_cursor_;
        //shassis_support_ptr           shassis_;

        phys::aircraft::control_ptr     phys_aircraft_;
        cg::transform_4                 body_transform_inv_;
        bool                            on_ground_;
        //fms::air_config_t             cfg_;
        double                          prediction_;

        cg::geo_point_3                 desired_position_;
        quaternion                      desired_orien_;
        bool                            tow_attached_;
        bool                            has_malfunction_;
        size_t                          zone_;
    };

}