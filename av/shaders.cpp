﻿#include "stdafx.h"
#include "shaders.h"


namespace shaders
{
    //template <typename S>
    //__forceinline S luminance_crt( color_t<S> const & col )
    //{
    //    return S(0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
    //}

    ////
    //template <typename S>
    //__forceinline S luminance_lcd( color_t<S> const & col )
    //{
    //    return S(0.2127f * col.r + 0.7152f * col.g + 0.0722f * col.b);
    //}

    namespace include_mat
    {

#define  SHADERS_GETTER                            \
        const char* get_shader(shader_t t)         \
        {                                          \
            if(t==VS)                              \
                return vs;                         \
            else if(t==FS)                         \
                return fs;                         \
            else                                   \
                return nullptr;                    \
        }                                          \


#define INCLUDE_FUNCS                                                                                    \
        STRINGIFY (                                                                                      \
        float saturate( const in float x )                                                               \
        {                                                                                                \
            return clamp(x, 0.0, 1.0);                                                                   \
        }                                                                                                \
                                                                                                         \
        float lerp(float a, float b, float w)                                                            \
        {                                                                                                \
            return a + w*(b-a);                                                                          \
        }                                                                                                \
                                                                                                         \
        vec3 hardlight( const in vec3 color, const in vec3 hl )                                          \
        {                                                                                                \
            vec3 hl_pos = step(vec3(0.0), hl);                                                           \
            return (vec3(1.0) - hl_pos) * color * (hl + vec3(1.0)) +                                     \
                hl_pos * mix(color, vec3(1.0), hl);                                                      \
        }                                                                                                \
                                                                                                         \
        float tex_detail_factor( const in vec2 tex_c_mod, const in float coef )                          \
        {                                                                                                \
            vec2 grad_vec = fwidth(tex_c_mod);                                                           \
            float detail_fac = exp(coef * dot(grad_vec, grad_vec));                                      \
            return detail_fac * (2.0 - detail_fac);                                                      \
        }                                                                                                \
                                                                                                         \
        float ramp_up( const in float x )                                                                \
        {                                                                                                \
            return x * fma(x, -0.5, 1.5);                                                                \
        }                                                                                                \
                                                                                                         \
        float ramp_down( const in float x )                                                              \
        {                                                                                                \
            return x * fma(x, 0.5, 0.5);                                                                 \
        }                                                                                                \
                                                                                                         \
                                                                                                         \
        )

#define INCLUDE_FOG_FUNCS                                                                                        \
    STRINGIFY (                                                                                                  \
                float fog_decay_factor( const in vec3 view_pos )                                                 \
                {                                                                                                \
                    return exp(-/*fog_params*/SceneFogParams.a * dot(view_pos, view_pos));                       \
                }                                                                                                \
                vec3 apply_scene_fog( const in vec3 view_pos, const in vec3 color )                              \
                {                                                                                                \
                    vec3 view_vec_fog = (mat3(viewworld_matrix) * view_pos) * vec3(1.0, 1.0, 0.8);               \
                    return mix(textureCube(envTex, view_vec_fog).rgb, color, fog_decay_factor(view_vec_fog));    \
                    /*return mix(textureLod(envTex, view_vec_fog, 3.0).rgb, color, fog_decay_factor(view_vec_fog));*/   \
                }                                                                                                \
                                                                                                                 \
                vec3 apply_clear_fog( const in vec3 view_pos, const in vec3 color )                              \
                {                                                                                                \
                    return mix(/*fog_params*/SceneFogParams.rgb, color, fog_decay_factor(view_pos));             \
                }                                                                                                \
                                                                                                                 \
                )


#define INCLUDE_VS                                                                                     \
    STRINGIFY (                                                                                        \
\n    float luminance_crt( const in vec4 col )                                                         \
\n    {                                                                                                \
\n        const vec4 crt = vec4(0.299, 0.587, 0.114, 0.0);                                             \
\n        return dot(col,crt);                                                                         \
\n    }                                                                                                \
\n                                                                                                     \
\n      uniform mat4      shadow0_matrix;                                                              \
\n                                                                                                     \
\n        float PCF4E(sampler2DShadow depths,vec4 stpq,ivec2 size){                                    \
\n            float result = 0.0;                                                                      \
\n            int   count = 0;                                                                         \
\n            for(int x=-size.x; x<=size.x; x++){                                                      \
\n                for(int y=-size.y; y<=size.y; y++){                                                  \
\n                    count++;                                                                         \
\n                    result += shadow2DProjOffset(depths, stpq, ivec2(x,y)).r;                        \
\n                }                                                                                    \
\n            }                                                                                        \
\n            return result/count;                                                                     \
        }                                                                                              \
                                                                                                       \
\n        float PCF4(sampler2DShadow depths,vec4 stpq,ivec2 size){                                     \
\n            float result = 0.0;                                                                      \
\n            result += shadow2DProjOffset(depths, stpq, ivec2(0,-1)).r;                               \
\n            result += shadow2DProjOffset(depths, stpq, ivec2(0,1)).r;                                \
\n            result += shadow2DProjOffset(depths, stpq, ivec2(1,0)).r;                                \
\n            result += shadow2DProjOffset(depths, stpq, ivec2(-1,0)).r;                               \
\n            return result*.25;                                                                       \
    }                                                                                                  \
                                                                                                       \
\n        float PCF(sampler2DShadow depths,vec4 stpq,ivec2 size){                                      \
\n            return shadow2DProj(depths, stpq).r;                                          \
    }                                                                                                  \
      const ivec2 pcf_size = ivec2(1,1);                                                               \
                                                                                                       \
     vec4  get_shadow_coords(vec4 posEye, int index)                                                   \
     {                                                                                                 \
         return vec4(dot( posEye, gl_EyePlaneS[index]),dot( posEye, gl_EyePlaneT[index] ),dot( posEye, gl_EyePlaneR[index]),dot( posEye, gl_EyePlaneQ[index] ) );  \
     }                                                                                                  \
                                                                                                        \
     uniform sampler2D baseTexture;                                                                     \
     uniform int baseTextureUnit;                                                                       \
     uniform sampler2DShadow shadowTexture0;                                                            \
     uniform int shadowTextureUnit0;                                                                    \
                                                                                                        \
     )

#define INCLUDE_SCENE_PARAM                                                                             \
     STRINGIFY (                                                                                        \
        uniform vec4 ambient;                                                                           \
        uniform vec4 diffuse;                                                                           \
        uniform vec4 specular;                                                                          \
     )

#define INCLUDE_UNIFORMS                                                                                \
    STRINGIFY (                                                                                         \
        uniform sampler2D           ViewLightMap;                                                       \
        uniform sampler2D           detailTex;                                                          \
        uniform samplerCube         envTex;                                                             \
        uniform sampler2DShadow     ShadowSplit0;                                                       \
        uniform sampler2DShadow     ShadowSplit1;                                                       \
        uniform sampler2DShadow     ShadowSplit2;                                                       \
        uniform sampler2D           ViewDecalMap;                                                       \
        uniform vec4                SceneFogParams;                                                     \
        )

#define INCLUDE_DL                                                                                       \
    STRINGIFY (                                                                                          \
    \
    const int nMaxLights = 130;                                                                           \
    \
    \
    uniform int LightsActiveNum;                                                                         \
    \
    uniform vec4 LightVSPosAmbRatio[nMaxLights];                                                         \
    uniform vec4 LightVSDirSpecRatio[nMaxLights];                                                        \
    uniform vec4 LightAttenuation[nMaxLights];                                                           \
    uniform vec3 LightDiffuse[nMaxLights];                                                               \
    \
    void ComputeDynamicLights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) \
   {                                                                                                     \
   int curLight = 0;                                                                                     \
   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      \
   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     \
   while (curLight < LightsActiveNum)                                                                    \
       {                                                                                                 \
       vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];                                            \
       vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];                                           \
       vec4 curAttenuation    = LightAttenuation[curLight];                                              \
       vec3 curDiffuse        = LightDiffuse[curLight];                                                  \
       \
       vec3 vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                        \
       float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                               \
       vec3 vDirToLight = vDistToLightInv * vVecToLight;                                                 \
       \
       float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                        \
       float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);               \
                                                                                                         \
       fTotalAtt *= clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);              \
       \
       if (fTotalAtt != 0.0)                                                                             \
           {                                                                                             \
           \
           float fDiffuseDot = dot(vDirToLight, vViewSpaceNormal);                                       \
           cAmbDiff += (fTotalAtt * (curVSPosAmbRatio.w + clamp(fDiffuseDot, 0.0, 1.0))) * curDiffuse;   \
           \
           float fSpecPower = clamp(dot(vReflVec, vDirToLight), 0.0, 1.0);                               \
           fSpecPower *= fSpecPower;                                                                     \
           fSpecPower *= fSpecPower;                                                                     \
           cSpecular += (fTotalAtt * curVSDirSpecRatio.w * fSpecPower) * curDiffuse;                     \
           }                                                                                             \
                                                                                                         \
                                                                                                         \
           ++curLight;                                                                                   \
       }                                                                                                 \
                                                                                                         \
                                                                                                         \
       return;                                                                                           \
   }                                                                                                     \
   )        

#define INCLUDE_DL2                                                                                      \
    STRINGIFY (                                                                                          \
                                                                                                         \
    const float PI = 3.14159265358979323846264;                                                          \
                                                                                                         \
   void compute_dynamic_lights( in vec3 vViewSpacePoint, in vec3 vViewSpaceNormal, in vec3 vReflVec, inout vec3 cAmbDiff, inout vec3 cSpecular ) \
   {                                                                                                     \
   int curLight = 0;                                                                                     \
   cAmbDiff = vec3(0.0f,0.0f,0.0f);                                                                      \
   cSpecular = vec3(0.0f,0.0f,0.0f);                                                                     \
   while (curLight < LightsActiveNum)                                                                    \
       {                                                                                                 \
       vec4 curVSPosAmbRatio  = LightVSPosAmbRatio[curLight];                                            \
       vec4 curVSDirSpecRatio = LightVSDirSpecRatio[curLight];                                           \
       vec4 curAttenuation    = LightAttenuation[curLight];                                              \
       vec3 curDiffuse        = LightDiffuse[curLight];                                                  \
                                                                                                         \
                                                                                                         \
        vec4 specular_ =  vec4(curDiffuse * curVSDirSpecRatio.w,1.0);                                    \
        vec3 vVecToLight = curVSPosAmbRatio.xyz - vViewSpacePoint;                                       \
        float vDistToLightInv = inversesqrt(dot(vVecToLight, vVecToLight));                              \
        vec3 vDirToLight = vDistToLightInv * vVecToLight;                                                \
                                                                                                         \
        float fAngleDot = dot(vDirToLight, curVSDirSpecRatio.xyz);                                       \
        float fTotalAtt = clamp(curAttenuation.z * fAngleDot + curAttenuation.w, 0.0, 1.0);              \
                                                                                                         \
        /*float*/ fTotalAtt = clamp(curAttenuation.x * vDistToLightInv + curAttenuation.y, 0.0, 1.0);    \
                                                                                                         \
        float intensity = 0.0;                                                                           \
        vec4 spec = vec4(0.0);                                                                           \
        vec3 ld = normalize(vVecToLight);                                                                \
        vec3 sd = normalize(vec3(-curVSDirSpecRatio.xyz));                                               \
                                                                                                         \
        if (dot(ld,sd) > cos(PI/4)) {                                                        \
                                                                                                         \
                    vec3 n = normalize(vViewSpaceNormal);                                                \
                    intensity = max(dot(n,ld), 0.0);                                                     \
                                                                                                         \
                        if (intensity > 0.0) {                                                           \
                                vec3 eye = normalize(vViewSpacePoint);                                   \
                                vec3 h = normalize(ld + eye);                                            \
                                float intSpec = max(dot(h,n), 0.0);                                      \
                                spec = specular_ * pow(intSpec, 1/*shininess*/);                         \
                            }                                                                            \
                }                                                                                        \
                                                                                                         \
           cAmbDiff += fTotalAtt *(intensity * curDiffuse + spec.rgb);                                   \
                                                                                                         \
                                                                                                         \
           ++curLight;                                                                                   \
       }                                                                                                 \
                                                                                                         \
       return;                                                                                           \
   }                                                                                                     \
   )  



    }                                                                                                  

    namespace plane_mat
    {

    const char* vs = {
		"#version 130 \n"
        "#extension GL_ARB_gpu_shader5 : enable \n"
        
        INCLUDE_VS

        STRINGIFY ( 
        attribute vec3 tangent;
        attribute vec3 binormal;
        varying   vec3 lightDir;

        out block
        {
            vec2 texcoord;
            vec3 normal;
            vec3 tangent;
            vec3 binormal;
            vec3 viewpos;
            vec4 shadow_view;
            vec4 lightmap_coord;
        } v_out;

        void main()
        {
            vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
            // mat3 rotation = mat3(tangent, binormal, normal);
            vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
            // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
            // lightDir = normalize(rotation * normalize(lightDir));
            lightDir = vec3(gl_LightSource[0].position.xyz);

            gl_Position    = ftransform();


            v_out.tangent   = tangent;
            v_out.binormal  = binormal;
            v_out.normal    = normal;
            v_out.viewpos   = vertexInEye.xyz;
            v_out.texcoord  = gl_MultiTexCoord1.xy;
            v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);

            //illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse); // FIXME Этот расчет должен быть в основной программе, а не для каждого фрагмента
        }
    )
    };


    const char* fs = {
        "#version 130 \n"
        "#extension GL_ARB_gpu_shader5 : enable \n"
		"#extension GL_ARB_gpu_shader_fp64 : enable \n"
        
        INCLUDE_UNIFORMS

        STRINGIFY ( 
    
        // layout(early_fragment_tests) in;

        mat4 viewworld_matrix;
        )

        INCLUDE_FUNCS
        INCLUDE_FOG_FUNCS
        INCLUDE_VS
        INCLUDE_DL
        INCLUDE_DL2
        INCLUDE_SCENE_PARAM

        STRINGIFY ( 

        
        uniform sampler2D colorTex;
        uniform sampler2D normalTex;
        varying   vec3 lightDir;
        //varying   float illum; 
        
        

        in block
        {
            vec2 texcoord;
            vec3 normal;
            vec3 tangent;
            vec3 binormal;
            vec3 viewpos;
            vec4 shadow_view;
            vec4 lightmap_coord;
        } f_in;


        void main (void)
        {
            // GET_SHADOW(f_in.viewpos, f_in);
            //#define GET_SHADOW(viewpos, in_frag)   
            float shadow = 1.0; 
            //if(ambient.a > 0.35)
               shadow = PCF(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4;


            //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

            //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
            //if (split_test.x) 
            //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
            //else if (split_test.y) 
            //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
            //else if (split_test.z) 
            //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

            //vec4  specular       = gl_LightSource[0].specular ;     // FIXME 
            //vec4  diffuse        = gl_LightSource[0].diffuse ;      // FIXME 
            //vec4  ambient        = gl_LightSource[0].ambient  ;      // FIXME 
            vec4  light_vec_view = vec4(lightDir,1);
            
            
            viewworld_matrix = mat4(vec4(1.0,0.0,0.0,0.0),vec4(0.0,1.0,0.0,0.0),vec4(0.0,0.0,1.0,0.0),vec4(0.0,0.0,0.0,1.0)); 
            //viewworld_matrix = gl_ModelViewMatrixInverse; 
            // 
            vec4 base = texture2D(colorTex, f_in.texcoord.xy);
            vec3 bump = fma(texture2D(normalTex, f_in.texcoord.xy).xyz, vec3(2.0), vec3(-1.0));
            //vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;
            //bump = normalize(bump * 2.0 - 1.0);
            vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
            vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord.xy, -1.0);
            float glass_factor = 1.0 - dif_tex_col.a;

            // get dist to point and normalized to-eye vector
            float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
            float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
            float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
            vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

            vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
            float normal_world_space_z = dot(view_up_vec, normal);


            float incidence_dot  = dot(to_eye, normal);
            float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
            vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
            vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
            float refl_min       = fma(glass_factor, 0.275, 0.125);
            float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
            float fresnel        = mix(refl_min, 0.97, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.25)); 

            float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
            float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
            vec3  pure_spec_color = specular.rgb * specular_val;
            float spec_compose_fraction = 0.35;


            // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
            vec3 cube_color = textureCube(envTex, refl_vec_world).rgb + pure_spec_color;


            vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
            // GET_LIGHTMAP(f_in.viewpos, f_in);
            // #define GET_LIGHTMAP(viewpos, in_frag) 
            // float height_world_lm      = in_frag.lightmap_coord.z; 
            // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
            // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
            // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

            //vec3 lightmap_color = vec3(0.6f,0.6f,0.6f); // FIXME dummy code
            vec3  light_res;  
            vec3  vLightsSpecAddOn; 
            /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

            vec3 lightmap_color = light_res; 

            float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
            non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

            float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
            vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
            vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
            float night_factor = step(ambient.a, 0.35);
            vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);
            //ALFA-TEST// gl_FragColor = vec4( glass_factor,0,0,1.0f);
            //LIGHT_VIEW_TEST//gl_FragColor = vec4(lightDir,1.0);    
            // gl_FragColor = vec4( result,1.0);
            gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
            //gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), base.a);  // for dynamic rotor
            
        }
    )

    };

    SHADERS_GETTER

     AUTO_REG_NAME(plane, shaders::plane_mat::get_shader)

    } // ns plane_mat

    namespace rotor_mat
    {

        const char* vs = {  
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 

            varying   vec3  lightDir;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                lightDir = vec3(gl_LightSource[0].position.xyz);;
                gl_Position = ftransform();

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);

            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "

            INCLUDE_UNIFORMS

            STRINGIFY ( 

            mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS

            INCLUDE_FOG_FUNCS

            INCLUDE_VS

            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D       colorTex;
            uniform sampler2D       nightTex;
            varying   vec3          lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;

            void main (void)
            {
                float shadow = 1.0; 
                if(ambient.a > 0.35)
                    shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4;

                vec4  light_vec_view = vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;


                vec3 normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float n_dot_l = shadow * saturate(fma(dot(normal, light_vec_view.xyz), 0.5, 0.5));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb;

                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                //gl_FragColor = vec4(1.0,0.0,0,1.0);
            }

            )

        };   


        SHADERS_GETTER

        AUTO_REG_NAME(rotor, shaders::rotor_mat::get_shader)

    } // ns rotor_mat

    namespace default_mat 
    {
       const char* vs = {  
		   "#version 130 \n"
           "#extension GL_ARB_gpu_shader5 : enable \n"

           INCLUDE_VS

           STRINGIFY ( 
           attribute vec3 tangent;
           attribute vec3 binormal;
           varying   vec3 lightDir;

           out block
           {
               vec2 texcoord;
               vec3 normal;
               vec3 tangent;
               vec3 binormal;
               vec3 viewpos;
               vec4 shadow_view;
               vec4 lightmap_coord;
           } v_out;

           void main()
           {
               vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
               mat3 rotation = mat3(tangent, binormal, normal);
               vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
               // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
               // lightDir = normalize(rotation * normalize(lightDir));
               lightDir = vec3(gl_LightSource[0].position.xyz);
               gl_Position = ftransform();

               v_out.tangent   = tangent;
               v_out.binormal  = binormal;
               v_out.normal    = normal;
               v_out.viewpos   = vertexInEye.xyz;
               v_out.texcoord  = gl_MultiTexCoord1.xy;
               v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);
           }       
       )
       };


       const char* fs = {
       "#version 130 \n"
       "#extension GL_ARB_gpu_shader5 : enable \n "
        
       INCLUDE_UNIFORMS

       STRINGIFY ( 

           mat4 viewworld_matrix;
       )
       
       INCLUDE_FUNCS
       INCLUDE_FOG_FUNCS
       INCLUDE_VS
       INCLUDE_DL
       INCLUDE_DL2
       INCLUDE_SCENE_PARAM

       STRINGIFY ( 

           uniform sampler2D colorTex;
           uniform sampler2D normalTex;
           varying vec3 lightDir;

           in block
           {
               vec2 texcoord;
               vec3 normal;
               vec3 tangent;
               vec3 binormal;
               vec3 viewpos;
               vec4 shadow_view;
               vec4 lightmap_coord;
           } f_in;


           void main (void)
           {
               // GET_SHADOW(f_in.viewpos, f_in);
               //#define GET_SHADOW(viewpos, in_frag) 
               float shadow = 1.0; 
               if(ambient.a > 0.35)
                   shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4;

               //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

               /// shadow = shadow2DProj(ShadowSplit0, f_in.shadow_view);
               //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
               //if (split_test.x) 
               //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
               //else if (split_test.y) 
               //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
               //else if (split_test.z) 
               //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

               //vec4  specular       = gl_LightSource[0].specular  ;     // FIXME 
               //vec4  diffuse        = gl_LightSource[0].diffuse  ;      // FIXME 
               //vec4  ambient        = gl_LightSource[0].ambient  ;      // FIXME 
               vec4  light_vec_view = vec4(lightDir,1);

               viewworld_matrix = gl_ModelViewMatrixInverse;
               vec4 base = texture2D(colorTex, f_in.texcoord);
               vec3 bump = fma(texture2D(normalTex, f_in.texcoord).xyz, vec3(2.0), vec3(-1.0));
               //vec3 bump = texture2D(normalTex, f_in.texcoord).xyz;
               //bump = normalize(bump * 2.0 - 1.0);
               vec3  normal       = normalize(bump.x * f_in.tangent + bump.y * f_in.binormal + bump.z * f_in.normal);
               vec4  dif_tex_col  = texture2D(colorTex,f_in.texcoord, -1.0);
               float glass_factor = /*1.0 - dif_tex_col.a*/0;

               // get dist to point and normalized to-eye vector
               float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
               float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
               float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
               vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

               vec3 view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
               float normal_world_space_z = dot(view_up_vec, normal);


               float incidence_dot  = dot(to_eye, normal);
               float pow_fr         = pow(saturate(1.0 - incidence_dot), 3.0);
               vec3  refl_vec_view  = -to_eye + (2.0 * incidence_dot) * normal;
               vec3  refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
               float refl_min       = 0.10 + glass_factor * 0.30;
               float half_refl_z    = 0.5 * (refl_vec_world.z + normal_world_space_z);
               float fresnel        = mix(refl_min, 0.6, pow_fr) * fma(half_refl_z, 0.15, fma(glass_factor, 0.6, 0.)); 

               float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz));
               float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 44.0) * 0.9;
               vec3  pure_spec_color = specular.rgb * specular_val;
               float spec_compose_fraction = 0.35;


               // const vec3 cube_color = texture(Env, refl_vec_world).rgb + pure_spec_color;
               vec3 cube_color = textureCube(envTex, refl_vec_world).rgb + pure_spec_color;

               vec3 non_ambient_term = diffuse.rgb * n_dot_l + spec_compose_fraction * pure_spec_color;
               // GET_LIGHTMAP(f_in.viewpos, f_in);
               // #define GET_LIGHTMAP(viewpos, in_frag) 
               // float height_world_lm      = in_frag.lightmap_coord.z; 
               // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
               // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
               // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

               //vec3 lightmap_color = vec3(0.1f,0.1f,0.1f); // FIXME dummy code
               vec3  light_res;  
               vec3  vLightsSpecAddOn; 
               /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

               vec3 lightmap_color = light_res; 

               float up_dot_clamped = saturate(fma(normal_world_space_z, 0.55, 0.45));
               non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

               float ao_trick = fma(up_dot_clamped, 0.4, 0.6);
               vec3  composed_lighting = ao_trick * ambient.rgb + non_ambient_term;
               vec3  day_result = mix(composed_lighting * dif_tex_col.rgb, cube_color, fresnel) + (1.0 - spec_compose_fraction) * pure_spec_color;
               float night_factor = step(ambient.a, 0.35);
               vec3  result = mix(day_result, vec3(0.90, 0.90, 0.86), night_factor * glass_factor);

               gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);

           }
       )

       };   

       SHADERS_GETTER

       AUTO_REG_NAME(default, shaders::default_mat::get_shader)

    }  // ns default_mat

    namespace building_mat 
    {
        const char* vs = {  
			"#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"
            
            INCLUDE_VS

            STRINGIFY ( 
            
            varying   vec3  lightDir;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;
             
            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                //lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                //lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);;
                gl_Position = ftransform();

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);
            }       
            )
        };


        const char* fs = { 
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "
            
            INCLUDE_UNIFORMS

            STRINGIFY ( 

             mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D           colorTex;
            uniform sampler2D           nightTex;
            varying   vec3              lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;

            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //#define GET_SHADOW(viewpos, in_frag) 
                float shadow = 1.0; 
                if(ambient.a > 0.35)
                    shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4; 
                //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

                /// shadow = shadow2DProj(ShadowSplit0, f_in.shadow_view);
                //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
                //if (split_test.x) 
                //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
                //else if (split_test.y) 
                //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
                //else if (split_test.z) 
                //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

                //vec4  specular       = gl_LightSource[0].specular ;     // FIXME 
                //vec4  diffuse        = gl_LightSource[0].diffuse  ;      // FIXME 
                //vec4  ambient        = gl_LightSource[0].ambient ;      // FIXME 

                vec4  light_vec_view = vec4(lightDir,1);

                viewworld_matrix = gl_ModelViewMatrixInverse;
         

                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
                vec3  to_eye          = -dist_to_pnt_rcp * f_in.viewpos;

                vec3  normal = normalize(f_in.normal);
                float incidence_dot = dot(to_eye, normal);
                vec3  refl_vec_view = -to_eye + (2.0 * incidence_dot) * normal;

                vec3  view_up_vec = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float normal_world_space_z = dot(view_up_vec, normal);

                // diffuse color and glass factor (make windows color look darker)
                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                dif_tex_col.rgb *= fma(dif_tex_col.a, 0.6, 0.4);
                float glass_factor = 1.0 - dif_tex_col.a;

                // get diffuse and specular value
                float n_dot_l = ramp_up(shadow * saturate(dot(normal, light_vec_view.xyz)));
                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), 10.0) * 2.0;
                vec3 spec_color = specular.rgb * specular_val;

                // lightmaps
                vec3 non_ambient_term = n_dot_l * diffuse.rgb;

                // GET_LIGHTMAP(f_in.viewpos, f_in);
                // #define GET_LIGHTMAP(viewpos, in_frag) 
                // float height_world_lm      = in_frag.lightmap_coord.z; 
                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

                //vec3 lightmap_color = vec3(0.5f,0.5f,0.5f); // FIXME dummy code
                
                vec3  light_res;  
                vec3  vLightsSpecAddOn; 
                /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

                vec3 lightmap_color = light_res; 

                //    LIGHTMAP_BUILDING_HEIGHT_TRICK;
                float up_dot_clamped = saturate(fma(normal_world_space_z, 0.4, 0.6));
                non_ambient_term = max(lightmap_color * up_dot_clamped, non_ambient_term);

                // overall lighting
                vec3 light_color = ambient.rgb + non_ambient_term;

                // apply detail texture
                float detail_factor = dif_tex_col.a * tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.075);
                if (detail_factor > 0.01)
                    dif_tex_col.rgb = hardlight(dif_tex_col.rgb, detail_factor * fma(texture2D(detailTex, f_in.texcoord * 9.73f).rrr, vec3(0.5), vec3(-0.25)));

                vec3 day_result = light_color * dif_tex_col.rgb;
                vec3 night_tex = vec3(0.0f,0.0f,0.0f); // I'm not sure
                if (glass_factor > 0.25)
                {
                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
                    refl_vec_world.z = abs(refl_vec_world.z);
                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 2.0), 0.65, 0.35)) * fma(refl_vec_world.z, 0.15, 0.85);
                    vec3 cube_color = textureCube(envTex, refl_vec_world).rgb;
                    day_result = mix(day_result, cube_color, glass_factor * fresnel) + spec_color * glass_factor;
                    night_tex = texture2D(nightTex, f_in.texcoord).rgb;
                }

                float night_factor = step(ambient.a, 0.35);
                vec3 result = mix(day_result, night_tex,  night_factor * glass_factor ); // 
               
                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
                // gl_FragColor = vec4( result,1.0);  
                /// gl_FragColor =  mix(texture2D(colorTex,f_in.texcoord), texture2D(nightTex, f_in.texcoord),night_factor);
                //gl_FragColor = vec4( shadow,shadow,shadow,1.0);  
            }
            )

        };   

    SHADERS_GETTER

        AUTO_REG_NAME(building, shaders::building_mat::get_shader)

    }  // ns building_mat    

    

    namespace tree_mat 
    {
        const char* vs = {  
			"#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 

            varying   vec3  lightDir;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                //lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                //lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);;
                gl_Position = ftransform();

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);

            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "
            
            INCLUDE_UNIFORMS

            STRINGIFY ( 

             mat4 viewworld_matrix;
            )
            
            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D       colorTex;
            uniform sampler2D       nightTex;
            varying   vec3          lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;

            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //#define GET_SHADOW(viewpos, in_frag) 
                float shadow = 1.0; 
                if(ambient.a > 0.35)
                    shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4;
                //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

                /// shadow = shadow2DProj(ShadowSplit0, f_in.shadow_view);
                //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
                //if (split_test.x) 
                //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
                //else if (split_test.y) 
                //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
                //else if (split_test.z) 
                //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

                //vec4  specular       = gl_LightSource[0].specular ;     // FIXME 
                //vec4  diffuse        = gl_LightSource[0].diffuse  ;      // FIXME 
                //vec4  ambient        = gl_LightSource[0].ambient  ;      // FIXME 
               vec4  light_vec_view = vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;


                vec3 normal = vec3(viewworld_matrix[0][2], viewworld_matrix[1][2], viewworld_matrix[2][2]);
                float n_dot_l = shadow * saturate(fma(dot(normal, light_vec_view.xyz), 0.5, 0.5));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb;

                // FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                // gl_FragColor = vec4( result, dif_tex_col.a);
                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                //gl_FragColor = vec4( shadow,shadow,shadow,1.0);  
            }

            )

        };   

        SHADERS_GETTER

        AUTO_REG_NAME(tree, shaders::tree_mat::get_shader)

    }  // ns tree_mat

    namespace ground_mat 
    {
        const char* vs = {
			 "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"
            
            INCLUDE_VS

            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;
            
            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                // lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);
                // lightDir = normalize(rotation * normalize(lightDir));
                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();

                
                v_out.normal    = normal;
                v_out.tangent   = tangent;
                v_out.binormal  = binormal;
                 
                v_out.viewpos   = vertexInEye.xyz;
                v_out.detail_uv = gl_Vertex.xy * 0.03;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);

           }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "
            
            INCLUDE_UNIFORMS

            STRINGIFY ( 

           mat4 viewworld_matrix;
            )
            
            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;

            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //#define GET_SHADOW(viewpos, in_frag) 
                float shadow = 1.0; 
                if(ambient.a > 0.35)
                    shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4; 
                //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

\n                ///shadow = shadow2DProj(ShadowSplit0, f_in.shadow_view);
\n                //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders);                                              
\n                //if (split_test.x)                                                                                                      
\n                //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
\n                //else if (split_test.y) 
\n                //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
\n                //else if (split_test.z) 
\n                //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);
\n
\n                //vec4  specular       = gl_LightSource[0].specular;     // FIXME 
\n                //vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
\n                //vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 

\n                vec4  light_vec_view = vec4(lightDir,1);
\n                viewworld_matrix = gl_ModelViewMatrixInverse;
\n                // FIXME dummy code
\n                // specular.a = 0; // it's not rainy day hallelujah
\n
\n                float rainy_value = 0.666 * specular.a;
\n
\n                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
\n                dif_tex_col *= fma(dif_tex_col, vec3(rainy_value,rainy_value,rainy_value)/*rainy_value.xxx*/, vec3(1.0 - rainy_value));
\n                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.02);
\n                vec3 normal_noise = vec3(0.0);
\n                if (detail_factor > 0.01)
\n                {
\n                    normal_noise = detail_factor * fma(texture2D(detailTex, f_in.detail_uv).rgb, vec3(0.6), vec3(-0.3));
\n                    dif_tex_col = hardlight(dif_tex_col, normal_noise.ggg);
\n                }
\n
\n                vec3 normal = normalize(0.8 * f_in.normal + (normal_noise.x * f_in.tangent + normal_noise.y * f_in.binormal));
\n                float n_dot_l = saturate(dot(normal, light_vec_view.xyz));
\n
\n
                // get dist to point and normalized to-eye vector
\n                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
\n
\n                // reflection vector
\n                float incidence_dot = dot(-to_pnt, normal);
\n                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);
\n
\n                // specular
\n                float specular_val = pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 3.0, 3.0)) * fma(rainy_value, 0.8, 0.3);
\n                vec3 specular_color = specular_val * specular.rgb;
\n
                // GET_LIGHTMAP(f_in.viewpos, f_in);
                // #define GET_LIGHTMAP(viewpos, in_frag) 
                // float height_world_lm      = in_frag.lightmap_coord.z; 
                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  
                
                vec3  light_res;  
                vec3  vLightsSpecAddOn; 
                /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

\n               vec3 lightmap_color = light_res; //vec3(0.0f,0.0f,0.0f); // FIXME dummy code
\n
\n                // LIGHTMAP_SHADOW_TRICK(shadow);
\n               vec3 non_ambient_term = max(lightmap_color, shadow * (diffuse.rgb * n_dot_l + specular_color));
\n
\n                // result
\n                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
\n                // reflection when rain
\n                if (rainy_value >= 0.01)
\n                {
\n                    vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
\n                    float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.25, 0.05));
\n                    vec3 cube_color = textureCube(envTex, refl_vec_world).rgb;
\n                    result = mix(result, lightmap_color + cube_color + specular_color, fresnel * rainy_value);
\n                }
\n
\n                //gl_FragColor = vec4( result,1.0);
\n
\n                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
\n
\n                //gl_FragColor = vec4( shadow,shadow,shadow,1.0);  
\n            }
            )

        };   

        SHADERS_GETTER

        AUTO_REG_NAME(ground, shaders::ground_mat::get_shader)
        AUTO_REG_NAME(sea, shaders::ground_mat::get_shader)
        AUTO_REG_NAME(mountain, shaders::ground_mat::get_shader)

    }  // ns ground_mat

    namespace concrete_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;
            
            mat4 decal_matrix;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 tangent;
                vec3 binormal;
                vec3 viewpos;
                vec2 detail_uv;
                vec4 shadow_view;
                vec4 lightmap_coord;
                vec4 decal_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;

                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();

                v_out.normal    = normal;
                v_out.tangent   = tangent;
                v_out.binormal  = binormal;

                v_out.viewpos   = vertexInEye.xyz;
                // v_out.detail_uv = position.xy * 0.045;
                v_out.detail_uv = gl_Vertex.xy * 0.045; // FIXME dont no how
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                
                // SAVE_DECAL_VARYINGS_VP
                v_out.decal_coord = (decal_matrix * vec4(v_out.viewpos,1.0)).xyzw;

                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);
                
            }       
            )
        };

        const char* fs = { 
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "

            INCLUDE_UNIFORMS

            STRINGIFY ( 

\n
\n            mat4 viewworld_matrix;
\n            )
            
              INCLUDE_FUNCS
              INCLUDE_FOG_FUNCS
              INCLUDE_VS
              INCLUDE_SCENE_PARAM
              INCLUDE_DL
              INCLUDE_DL2

              STRINGIFY ( 
\n
\n            uniform sampler2D colorTex;
\n            varying vec3 lightDir;
\n
\n            in block
\n            {
\n                vec2 texcoord;
\n                vec3 normal;
\n                vec3 tangent;
\n                vec3 binormal;
\n                vec3 viewpos;
\n                vec2 detail_uv;
\n                vec4 shadow_view;
\n                vec4 lightmap_coord;
\n                vec4 decal_coord;
\n            } f_in;
\n            
              out vec4  aFragColor;

\n            void main (void)
\n            {
\n                // GET_SHADOW(f_in.viewpos, f_in);
\n                //#define GET_SHADOW(viewpos, in_frag) 
                  float shadow = 1.0; 
                  if(ambient.a > 0.35)
                      shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4; 
                  //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);
                  // color *= mix( colorAmbientEmissive * (1 - illum), gl_Color , f_in.shadow_view ); 
\n                //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
\n                //if (split_test.x) 
\n                //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
\n                //else if (split_test.y) 
\n                //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
\n                //else if (split_test.z) 
\n                //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);
\n
\n                //vec4  specular       = gl_LightSource[0].specular;     // FIXME 
\n                //vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
\n                //vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 
\n
\n                vec4  light_vec_view = vec4(lightDir,1);
\n                viewworld_matrix = gl_ModelViewMatrixInverse;
\n                // FIXME dummy code
\n                //specular.a = 0; // it's not rainy day hallelujah
\n
\n                float rainy_value = specular.a;
\n
\n                vec3 dif_tex_col = texture2D(colorTex, f_in.texcoord).rgb;
\n                float tex_mix_val = rainy_value * 0.7;
\n                dif_tex_col *= fma(dif_tex_col, vec3(tex_mix_val,tex_mix_val,tex_mix_val)/*tex_mix_val.xxx*/, vec3(1.0 - tex_mix_val));
\n                float detail_factor = tex_detail_factor(f_in.texcoord * textureSize2D(colorTex, 0), -0.015);
\n                vec3 concrete_noise = vec3(0.0);
\n                if (detail_factor > 0.01)
\n                {
\n                    concrete_noise = detail_factor * fma(texture2D(detailTex, f_in.detail_uv).rgb, vec3(0.48), vec3(-0.24));
\n                    dif_tex_col = hardlight(dif_tex_col, concrete_noise.bbb);
\n                }
\n
\n                // FIXME
\n                // APPLY_DECAL(f_in, dif_tex_col);
\n                vec4 decal_data = textureProj(ViewDecalMap, f_in.decal_coord).rgba; 
\n                // dif_col.rgb = fma(dif_col.rgb, vec3(1.0 - decal_data.a), decal_data.rgb);        // FIXME
\n                // decal_data.a = 1.0; //FIXME Dummy code 
\n
\n                // get dist to point and normalized to-eye vector
\n                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
\n                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
\n                float dist_to_pnt     = dist_to_pnt_rcp * dist_to_pnt_sqr;
\n                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
\n
\n                // reflection vector
\n
\n                vec3 normal = normalize(f_in.normal + (concrete_noise.x * f_in.tangent + concrete_noise.y * f_in.binormal) * (1.0 - decal_data.a));
\n                float incidence_dot = dot(-to_pnt, normal);
\n                vec3 refl_vec_view = fma(normal, vec3(2.0 * incidence_dot), to_pnt);
\n                
\n                // diffuse term
\n                float n_dot_l = shadow * saturate(dot(normal, light_vec_view.xyz)) * fma(rainy_value, -0.7, 1.0);
\n
\n                // specular
\n                float specular_val = shadow * pow(saturate(dot(refl_vec_view, light_vec_view.xyz)), fma(rainy_value, 5.0, 1.5)) * fma(rainy_value, 0.9, 0.7);
\n                vec3 specular_color = specular_val * specular.rgb;
\n
\n                  // Apply spot lights
\n                  vec3 vLightsAddOn;
\n                  vec3 vLightsSpecAddOn;
\n                  //ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, vLightsAddOn, vLightsSpecAddOn);
\n                  //result += vLightsAddOn;//vLightsAddOn ;//* saturate(0.85 - 0.55 * saturate(FresnelDamping));
\n                  // SpecColor += vLightsSpecAddOn;

//\n                  float PI = 3.14159265358979323846264;
//\n                  vec4 curVSPosAmbRatio  = LightVSPosAmbRatio [0];
//\n                  vec4 curVSDirSpecRatio = LightVSDirSpecRatio[0];
//\n                  vec4 curAttenuation    = LightAttenuation   [0];
//\n                  vec3 curDiffuse        = LightDiffuse       [0];
//\n 
//\n                  float intensity = 0.0;
//\n                  vec4 spec = vec4(0.0);
//\n                  vec4 specular_ =  vec4(curDiffuse * curVSDirSpecRatio.w,1.0);
//
//\n                  vec3 ld = normalize(curVSPosAmbRatio.xyz - f_in.viewpos.xyz);
//\n                  vec3 sd = normalize(vec3(-curVSDirSpecRatio.xyz));  
//\n
//\n                  // inside the cone?
//\n                 if (dot(ld,sd) > cos(PI/5)/*l_spotCutOff*/) {
//\n 
//\n                     vec3 n = f_in.normal;
//\n                     intensity = max(dot(n,ld), 0.0);
//\n 
//\n                     if (intensity > 0.0) {
//\n                          vec3 eye = normalize(f_in.viewpos);
//\n                          vec3 h = normalize(ld + eye);
//\n                          float intSpec = max(dot(h,n), 0.0);
//\n                          spec = specular_ * pow(intSpec, 0.1/*shininess*/);
//\n                      }
//\n                  }
//\n                  
//\n                vec3  light_res = intensity * curDiffuse + spec.rgb;// max(intensity * curDiffuse + spec.rgb, ambient.rgb);
                  
                  vec3  light_res;  
                  
                  /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

\n                // GET_LIGHTMAP(f_in.viewpos, f_in);
\n                // #define GET_LIGHTMAP(viewpos, in_frag) 
\n                // float height_world_lm      = in_frag.lightmap_coord.z; 
\n                // vec4  lightmap_data        = textureProj(ViewLightMap, in_frag.lightmap_coord).rgba; 
\n                // float lightmap_height_fade = clamp(fma(lightmap_data.w - height_world_lm, 0.4, 0.75), 0.0, 1.0); 
\n                // vec3  lightmap_color       = lightmap_data.rgb * lightmap_height_fade;  

 \n               vec3 lightmap_color = light_res; // FIXME dummy code
 \n
 \n               // FIXME
 \n               // LIGHTMAP_SHADOW_TRICK(shadow);
 \n
 \n               vec3 non_ambient_term = max(lightmap_color, diffuse.rgb * n_dot_l + specular_color);
 \n
 \n               // result
 \n               vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;
 \n               // reflection when rain
 \n               if (rainy_value >= 0.01)
 \n               {
 \n                   vec3 refl_vec_world = mat3(viewworld_matrix) * refl_vec_view;
 \n                   float fresnel = saturate(fma(pow(1.0 - incidence_dot, 5.0), 0.45, 0.05));
 \n                   vec3 cube_color = textureCube(envTex, refl_vec_world).rgb;
 \n                   result = mix(result, lightmap_color + cube_color, fresnel * rainy_value) + (fma(fresnel, 0.5, 0.5) * rainy_value) * specular_color;
 \n               }
 \n
                  // result += light_res;


 \n 
 \n              // aFragColor = vec4( result,1.0);
                 aFragColor = vec4(apply_scene_fog(f_in.viewpos, result), 1.0);
              }
            )
 
        };   
        
        const char* vs_test = {

            STRINGIFY ( 
            vec4 Ambient;
            vec4 Diffuse;
            vec4 Specular;


            void pointLight(in int i, in vec3 normal, in vec3 eye, in vec3 ecPosition3)
            {
                float nDotVP;       // normal . light direction
                float nDotHV;       // normal . light half vector
                float pf;           // power factor
                float attenuation;  // computed attenuation factor
                float d;            // distance from surface to light source
                vec3  VP;           // direction from surface to light position
                vec3  halfVector;   // direction of maximum highlights

                // Compute vector from surface to light position
                VP = vec3 (gl_LightSource[i].position) - ecPosition3;

                // Compute distance between surface and light position
                d = length(VP);

                // Normalize the vector from surface to light position
                VP = normalize(VP);

                // Compute attenuation
                attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
                    gl_LightSource[i].linearAttenuation * d +
                    gl_LightSource[i].quadraticAttenuation * d * d);

                halfVector = normalize(VP + eye);

                nDotVP = max(0.0, dot(normal, VP));
                nDotHV = max(0.0, dot(normal, halfVector));

                if (nDotVP == 0.0)
                {
                    pf = 0.0;
                }
                else
                {
                    pf = pow(nDotHV, gl_FrontMaterial.shininess);

                }
                Ambient  += gl_LightSource[i].ambient * attenuation;
                Diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;
                Specular += gl_LightSource[i].specular * pf * attenuation;
            }

            void directionalLight(in int i, in vec3 normal)
            {
                float nDotVP;         // normal . light direction
                float nDotHV;         // normal . light half vector
                float pf;             // power factor

                nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position))));
                nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector)));

                if (nDotVP == 0.0)
                {
                    pf = 0.0;
                }
                else
                {
                    pf = pow(nDotHV, gl_FrontMaterial.shininess);

                }
                Ambient  += gl_LightSource[i].ambient;
                Diffuse  += gl_LightSource[i].diffuse * nDotVP;
                Specular += gl_LightSource[i].specular * pf;
            }

            vec3 fnormal(void)
            {
                //Compute the normal 
                vec3 normal = gl_NormalMatrix * gl_Normal;
                normal = normalize(normal);
                return normal;
            }

            void flight(in vec3 normal, in vec4 ecPosition, float alphaFade)
            {
                vec4 color;
                vec3 ecPosition3;
                vec3 eye;

                ecPosition3 = (vec3 (ecPosition)) / ecPosition.w;
                eye = vec3 (0.0, 0.0, 1.0);

                // Clear the light intensity accumulators
                Ambient  = vec4 (0.0);
                Diffuse  = vec4 (0.0);
                Specular = vec4 (0.0);

                pointLight(0, normal, eye, ecPosition3);

                pointLight(1, normal, eye, ecPosition3);

                directionalLight(2, normal);

                color = gl_FrontLightModelProduct.sceneColor +
                    Ambient  * gl_FrontMaterial.ambient +
                    Diffuse  * gl_FrontMaterial.diffuse;
                color += Specular * gl_FrontMaterial.specular;
                color = clamp( color, 0.0, 1.0 );
                gl_FrontColor = color;

                gl_FrontColor.a *= alphaFade;
            }


            void main (void)
            {
                vec3  transformedNormal;
                float alphaFade = 1.0;

                // Eye-coordinate position of vertex, needed in various calculations
                vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;

                // Do fixed functionality vertex transform
                gl_Position = ftransform();
                transformedNormal = fnormal();
                flight(transformedNormal, ecPosition, alphaFade);
            }
            )
        };
        
        const char* vs_test2 = { 
            INCLUDE_VS

            STRINGIFY ( 
            uniform mat4  shadowMatrix;                                \n 
            uniform mat4  refMatrix;

            void main (void)
            {
               vec4 posEye    =  gl_ModelViewMatrix * gl_Vertex;
               gl_Position    =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; //ftransform();                       
               gl_TexCoord[0] =  gl_MultiTexCoord1;                  
               gl_FrontColor  =  gl_Color;                          
               gl_BackColor   =  gl_Color;                          

               //gl_TexCoord[1].s = dot( posEye, gl_EyePlaneS[5] );
               //gl_TexCoord[1].t = dot( posEye, gl_EyePlaneT[5] );
               //gl_TexCoord[1].p = dot( posEye, gl_EyePlaneR[5] );
               //gl_TexCoord[1].q = dot( posEye, gl_EyePlaneQ[5] );
               
               gl_TexCoord[1] = get_shadow_coords(posEye, 5);

               //gl_TexCoord[1] =   shadowMatrix * refMatrix * gl_Vertex  ;   //  shadowMatrix * gl_ModelViewMatrix *  refMatrix
               
            }
            )
        };

        const char* fs_test = { 

            INCLUDE_VS

            STRINGIFY ( 

            
            float texture2DCompare(sampler2D depths, vec2 uv, float compare){
                float depth = texture2D(depths, uv).r;
                return step(compare, depth);
             } 

            float PCF2(sampler2D depths, vec2 size, vec2 uv, float compare){
                float result = 0.0;
                for(int x=-2; x<=2; x++){
                    for(int y=-2; y<=2; y++){
                        vec2 off = vec2(x,y)/size;
                        result += texture2DCompare(depths, uv+off, compare);
                    }
                }
                return result/25.0;
             }


            void main(void)                                                         \n
            {                                                                       \n
                vec4 colorAmbientEmissive = vec4(1.0);//gl_FrontLightModelProduct.sceneColor;       \n
                vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );  // baseTextureUnit             \n

                float shadow = PCF2(shadowTexture0, gl_TexCoord[1],pcf_size); // shadowTextureUnit0
            
                float illum = luminance_crt(gl_LightSource[0].ambient + gl_LightSource[0].diffuse);

                color *= mix( colorAmbientEmissive * (1 - illum), gl_Color , shadow /*shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r*/ );     \n
                gl_FragColor = color;                                                                                                \n
                //gl_FragColor = gl_TexCoord[1]; \n
                //gl_FragColor = colorAmbientEmissive*shadow; \n
            }
            )
        };

        SHADERS_GETTER
        
        AUTO_REG_NAME(concrete, shaders::concrete_mat::get_shader)

    }  // ns concrete_mat

    namespace railing_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } v_out;

            void main()
            {
                vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
                // mat3 rotation = mat3(tangent, binormal, normal);
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;

                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();

                v_out.normal    = normal;
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;
                v_out.shadow_view = get_shadow_coords(vertexInEye, shadowTextureUnit0);

            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "
            
            INCLUDE_UNIFORMS

            STRINGIFY ( 

            mat4 viewworld_matrix;
            )

            INCLUDE_FUNCS
            INCLUDE_FOG_FUNCS
            INCLUDE_VS
            INCLUDE_DL
            INCLUDE_DL2
            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
                vec4 shadow_view;
                vec4 lightmap_coord;
            } f_in;


            void main (void)
            {
                // GET_SHADOW(f_in.viewpos, f_in);
                //#define GET_SHADOW(viewpos, in_frag) 
                float shadow = 1.0; 
                if(ambient.a > 0.35)
                    shadow = PCF4E(shadowTexture0, f_in.shadow_view,pcf_size) * ambient.a * 0.4;  
                //float shadow = shadow2DProj(shadowTexture0, f_in.shadow_view);

                ///shadow = shadow2DProj(ShadowSplit0, f_in.shadow_view);
                //bvec4 split_test = lessThanEqual(vec4(-viewpos.z), shadow_split_borders); 
                //if (split_test.x) 
                //    shadow = textureProj(ShadowSplit0, shadow0_matrix * in_frag.shadow_view); 
                //else if (split_test.y) 
                //    shadow = textureProj(ShadowSplit1, shadow1_matrix * in_frag.shadow_view); 
                //else if (split_test.z) 
                //    shadow = textureProj(ShadowSplit2, shadow2_matrix * in_frag.shadow_view);

                //vec4  specular       = gl_LightSource[0].specular;     // FIXME 
                //vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
                //vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 

                vec4  light_vec_view = vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;
                // FIXME dummy code

                vec3 normal = normalize(f_in.normal);
                float n_dot_l = saturate(fma(dot(normal, light_vec_view.xyz), 0.6, 0.4));

                // get dist to point and normalized to-eye vector
                float dist_to_pnt_sqr = dot(f_in.viewpos, f_in.viewpos);
                float dist_to_pnt_rcp = inversesqrt(dist_to_pnt_sqr);
                float dist_to_pnt = dist_to_pnt_rcp * dist_to_pnt_sqr;

                vec3 to_pnt = dist_to_pnt_rcp * f_in.viewpos;
                vec3 half_v = normalize(-to_pnt + light_vec_view.xyz);
                float specular_val = pow(saturate(dot(half_v, normal)), 8.0) * 0.5;

                vec3 non_ambient_term = shadow * (diffuse.rgb * n_dot_l + specular.xyz * specular_val);
                // GET_LIGHTMAP(f_in.viewpos, f_in);
                //vec3 lightmap_color = vec3(0.0f,0.0f,0.0f); // FIXME dummy code
                
                vec3  light_res;  
                vec3  vLightsSpecAddOn; 
                /*compute_dynamic_lights*/ComputeDynamicLights(f_in.viewpos.xyz, f_in.normal, f_in.normal, light_res, vLightsSpecAddOn);

                vec3 lightmap_color = light_res; 

                non_ambient_term = max(lightmap_color, non_ambient_term);

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + non_ambient_term) * dif_tex_col.rgb;

                // gl_FragColor = vec4( result,dif_tex_col.a);
                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                //gl_FragColor = vec4( shadow,shadow,shadow,1.0); 
                
            }
            )

        };   

        SHADERS_GETTER

        AUTO_REG_NAME(railing, shaders::railing_mat::get_shader)

    }  // ns railing_mat

    namespace panorama_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            varying   vec3 lightDir;

            out block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
            } v_out;

            void main()
            {
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;

                lightDir = vec3(gl_LightSource[0].position.xyz);
                gl_Position = ftransform();

                v_out.normal = vec3(gl_ModelViewMatrixInverse[0][2], gl_ModelViewMatrixInverse[1][2], gl_ModelViewMatrixInverse[2][2]);
                v_out.viewpos   = vertexInEye.xyz;
                v_out.texcoord  = gl_MultiTexCoord1.xy;

            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n "
            
            INCLUDE_UNIFORMS

            STRINGIFY ( 

            uniform vec4                fog_params; 
            mat4 viewworld_matrix;

            )

            INCLUDE_FUNCS

            INCLUDE_FOG_FUNCS

            INCLUDE_VS

            INCLUDE_SCENE_PARAM

            STRINGIFY ( 

            uniform sampler2D colorTex;
            varying vec3 lightDir;

            in block
            {
                vec2 texcoord;
                vec3 normal;
                vec3 viewpos;
            } f_in;

            void main (void)
            {
                //vec4  specular       = gl_LightSource[0].specular;     // FIXME 
                //vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
                //vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 

                vec4  light_vec_view =  vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;
                // FIXME dummy code

                vec3 normal = normalize(f_in.normal);
                float n_dot_l = saturate(fma(dot(normal, light_vec_view.xyz), 0.75, 0.25));

                vec4 dif_tex_col = texture2D(colorTex, f_in.texcoord);
                vec3 result = (ambient.rgb + diffuse.rgb * n_dot_l) * dif_tex_col.rgb * 0.5;
                
                gl_FragColor = vec4(apply_scene_fog(f_in.viewpos, result), dif_tex_col.a);
                // gl_FragColor = vec4( result,dif_tex_col.a);
                // gl_FragColor = vec4(SceneFogParams.rgb * (1-SceneFogParams.a),dif_tex_col.a);
            }
            )

        };   

        SHADERS_GETTER

        AUTO_REG_NAME(panorama, shaders::panorama_mat::get_shader)

    }  // ns panorama_mat

    namespace sky_fog_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 
            attribute vec3 tangent;
            attribute vec3 binormal;
            //varying   vec3 lightDir;

            out block
            {
                vec3 pos;
            } v_out;

            void main()
            {
                vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;
                
                v_out.pos = gl_Vertex.xyz;
				    // perform conversion to post-projective space

				vec3 vLocalSpaceCamPos = gl_ModelViewMatrixInverse[3].xyz;
				gl_Position = gl_ModelViewProjectionMatrix * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);

				gl_Position.z = 0.0;
                //gl_Position.z = gl_Position.w;
            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable  \n"
            
            INCLUDE_SCENE_PARAM

            INCLUDE_UNIFORMS

            INCLUDE_FUNCS

            STRINGIFY ( 


            uniform vec4                fog_params;     
            uniform vec4                SkyFogParams;  
            

            mat4 viewworld_matrix;                       
			                                            
            )

            STRINGIFY ( 
            
                            
            const float fTwoOverPi = 2.0 / 3.141593;     
            
            // varying vec3 lightDir;

            in block                                    
            {                                           
                vec3 pos;                               
            } f_in;                                     
			
            void main (void)                              
            {
                //vec4  specular       = gl_LightSource[0].specular;     // FIXME 
                //vec4  diffuse        = gl_LightSource[0].diffuse;      // FIXME 
                //vec4  ambient        = gl_LightSource[0].ambient;      // FIXME 
				
			
                // vec4  light_vec_view = vec4(lightDir,1);
                viewworld_matrix = gl_ModelViewMatrixInverse;       
                
                // get point direction
                vec3 vPnt = normalize(f_in.pos.xyz);                

                // fog color
                float fHorizonFactor = fTwoOverPi * acos(max(vPnt.z, 0.0)); 
                //if (vPnt.z<0)
                //    discard;

                // simulate fogging here based on input density
                float fFogDensity = SkyFogParams.a; 
                float fFogHeightRamp = fFogDensity * (2.0 - fFogDensity); 
                float fFogDensityRamp = fFogDensity; 
                //float fFogFactor = mix(pow(fHorizonFactor, 40.0 - 37.0 * fFogHeightRamp), 1.0, fFogDensityRamp);
                float fFogFactor = lerp(pow(fHorizonFactor, 30.0 - 25.0 * fFogHeightRamp), 1.0, fFogDensityRamp); 
                // pow(fHorizonFactor, 35.0 - 34.0 * fFogHeightRamp); 

                // make fogging
                gl_FragColor = vec4(SkyFogParams.rgb, fFogFactor); 

                //gl_FragColor = vec4( result,dif_tex_col.a);
                //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
            }
            )

        };   

        SHADERS_GETTER

        AUTO_REG_NAME(sky, shaders::sky_fog_mat::get_shader)

    }  // ns sky_fog_mat

    namespace clouds_mat 
    {
        const char* vs = {  
            "#extension GL_ARB_gpu_shader5 : enable \n"
            STRINGIFY ( 

            uniform mat4 MVP;
            out block
            {
                vec3 pos;
            } v_out;
            
            void main()
            {
                v_out.pos = gl_Vertex.xyz;
                mat4 im = inverse(gl_ModelViewMatrix*MVP);
                vec3 vLocalSpaceCamPos = im[3].xyz;//gl_ModelViewMatrixInverse[3].xyz;

               // gl_Position = gl_ModelViewProjectionMatrix   * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
                gl_Position =  gl_ModelViewProjectionMatrix * MVP * vec4(vLocalSpaceCamPos.xyz + gl_Vertex.xyz, 1.0);
                gl_Position.z = 0.0;
                //gl_Position.z = gl_Position.w;
            }       
            )
        };


        const char* fs = {
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable  \n"

            STRINGIFY ( 

            uniform sampler2D Clouds;

            // uniforms for sundisc and sunrays
            uniform vec3  frontColor;
            uniform vec3  backColor;
            uniform float density;
            
            
            )

            STRINGIFY ( 


            const float fOneOver2Pi = 0.5 / 3.141593;
            const float fTwoOverPi = 2.0 / 3.141593;   

            // varying vec3 lightDir;

            in block                                    
            {                                           
                vec3 pos;                               
            } f_in;                                     

            void main (void)                              
            {
               // get point direction
                vec3 vPnt = normalize(f_in.pos);

                // calculate texel coords based on polar angles
                vec2 vTexCoord = vec2(fOneOver2Pi * atan(vPnt.y, vPnt.x) + 0.5, fTwoOverPi * acos(abs(vPnt.z)));

                // get clouds color
                vec4 cl_color = textureLod(Clouds, vTexCoord, 0.0);

                // make fogging
                gl_FragColor = vec4(cl_color.rgb * frontColor, cl_color.a * density); 

                //gl_FragColor = vec4( result,dif_tex_col.a);
                //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
            }
            )

        };   

        SHADERS_GETTER
        
        AUTO_REG_NAME(clouds, shaders::clouds_mat::get_shader)

    }  // ns clouds_mat

    namespace  light_mat
    {

        const char* vs = { 
            INCLUDE_VS

            INCLUDE_UNIFORMS

            INCLUDE_FUNCS

            STRINGIFY ( 

            vec4 LightScreenSettings;
            
            void main (void)
            {
                LightScreenSettings = vec4(1.0,5.0,10000.0,40000.0);
                // constants here
                float ScreenClarity = LightScreenSettings.x;
                float VerticalScale = LightScreenSettings.y;
                float DistanceFadeOut = LightScreenSettings.z;
                float DistanceFog = LightScreenSettings.w;

                // position in view space
                gl_Position.xyz = gl_Vertex.xyz;
                // gl_Position = ftransform();

                // vis distances
                //float fDistSqr = dot(gl_Vertex.xyz, gl_Vertex.xyz);
                float fDistSqr = dot(gl_Position.xyz, gl_Position.xyz);
                float fDistInv = inversesqrt(fDistSqr);
                float fDist = fDistInv * fDistSqr;

                // get vis dist factor (1 - before 2/3*D, 0 - after 2*D)
                float fVisDistFactor = saturate(fDistInv * gl_MultiTexCoord0.y - 0.5);
                // make it be more sharp
                fVisDistFactor *= 2.0 - fVisDistFactor;

                // total fogging
                float fGlobalFogFactor = SceneFogParams.a * fDist;
                vec4 cDummy;
                float fLocalFogFactor = 0.0;//suppressVisibilityByLocalBanks(gl_Position.xyz, cDummy);
                float fTotalFogDistAtt = fGlobalFogFactor + fLocalFogFactor;

                // alpha based on fogging
                float fAlphaFogFactor = exp(-0.35 * fTotalFogDistAtt);
                float fAlphaSizeFogFactor = fAlphaFogFactor;
                fAlphaFogFactor *= 2.0 - fAlphaFogFactor;
                // size growing based on fogging
                // reflection size growing is prohibited
                float fSizeFogFactor = lerp(1.0 + 3.5 * step(VerticalScale, 1.0), 1.0, fAlphaSizeFogFactor);

                // global alpha fall-off (for reflections)
                float fAlphaDistFade = saturate(max((fDistInv * DistanceFadeOut - 1.0) * 0.25, 0.03));

                // real size, pixel screen size, magic vertical scale
                //gl_TexCoord[0].xyz = vec3(
                //    gl_MultiTexCoord0.x,
                //    ScreenClarity * fVisDistFactor * fSizeFogFactor,
                //    VerticalScale);

                // color (transp is also modulated by visible distance)
                gl_FrontColor = gl_Color;
                gl_FrontColor.a *= fVisDistFactor * fAlphaDistFade * fAlphaFogFactor;

            }
            )
        };

        const char* fs = { 

            INCLUDE_VS

            STRINGIFY ( 
            

            // uniform sampler2D texCulturalLight;

            void main(void)                                                         
            {                                                                       
                gl_FragColor = vec4(gl_Color.rgb, 1.0);//gl_Color.a * texture2D(texCulturalLight, gl_TexCoord[0].xy).r);                                               
            }

            )
        };

        SHADERS_GETTER

        AUTO_REG_NAME(simlight , shaders::light_mat::get_shader)
    }  // ns light_mat


    namespace  spot_mat
    {

        const char* vs = { 
            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            INCLUDE_UNIFORMS
            
            INCLUDE_FUNCS

            STRINGIFY ( 

            uniform mat4 mvp_matrix;

            attribute vec3 position;
            attribute vec3 from_l;
            attribute vec3 l_dir;
            attribute vec3 l_color;
            attribute dist_falloff;
            attribute vec2 cone_falloff;

            out block
            {
                vec3 from_l;
                flat vec3 l_dir;
                flat vec3 l_color;
                flat vec2 dist_falloff;
                flat vec2 cone_falloff;
            } v_out;

            void main()
            {
                gl_Position = mvp_matrix * vec4(position, 1.0);

                v_out.from_l = from_l;
                v_out.l_dir = l_dir;
                v_out.l_color = l_color;
                v_out.dist_falloff = dist_falloff;
                v_out.cone_falloff = cone_falloff;
            }

            )
        };

        const char* fs = { 

            "#version 130 \n"
            "#extension GL_ARB_gpu_shader5 : enable \n"

            INCLUDE_VS

            STRINGIFY ( 

            in block
            {
                vec3 from_l;
                flat vec3 l_dir;
                flat vec3 l_color;
                flat vec2 dist_falloff;
                flat vec2 cone_falloff;
            } f_in;

            out vec4 FragColor; 

            void main()
            {
                // get dist falloff
                const float dist_rcp = inversesqrt(dot(f_in.from_l, f_in.from_l));
                const vec3 from_l_nrm = dist_rcp * f_in.from_l;
                const float dist_atten = clamp(fma(dist_rcp, f_in.dist_falloff.x, f_in.dist_falloff.y), 0.0, 1.0);
                // get conical falloff
                const float angle_dot = dot(from_l_nrm, f_in.l_dir);
                const float angle_atten = clamp(fma(angle_dot, f_in.cone_falloff.x, f_in.cone_falloff.y), 0.0, 1.0);
                // diffuse-like term for planar surfaces
                //const float ndotl = clamp(fma(-from_l_nrm.z, 0.35, 0.65), 0.0, 1.0);
                // write color
                const float height_packed = -f_in.from_l.z;
                const float angledist_atten = angle_atten * dist_atten;
                const float angledist_atten_ramped = angledist_atten * (2.0 - angledist_atten);
                FragColor = vec4(f_in.l_color * (angledist_atten/* * ndotl*/), height_packed * angledist_atten_ramped);
            }

            )
        };

        SHADERS_GETTER

        AUTO_REG_NAME(spot , shaders::spot_mat::get_shader)

    }  // ns light_mat

}  // ns shaders
