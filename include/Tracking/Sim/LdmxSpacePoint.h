//TODO:: Covariance?!
//VarianceR is the variance in the more sensitive direction
//VarianceZ is the variance in the less sensitive direction

#include <cmath>

namespace ldmx {

    class LdmxSpacePoint  {
        
    public : 
        LdmxSpacePoint(float x, float y,
                       float z, float t,
                       int layer) {
            
            m_x = x;
            m_y = y;
            m_z = z;
            m_t = t;
            m_r = std::sqrt(m_x*m_x + m_y*m_y);
            m_layer = layer;
            m_id = -999;
            m_varianceR = 0.05;
            m_varianceZ = 0.250;
        }
        
        LdmxSpacePoint(float x,float y,float z,
                       float t,int layer,
                       float vR, float vZ,
                       int id) {
            m_x = x;
            m_y = y;
            m_z = z;
            m_t = t;
            m_varianceR = vR;
            m_varianceZ = vZ;
            m_r = std::sqrt(m_x*m_x + m_y*m_y);
            m_layer = layer;
            m_id = id;
        }

        /*
        LdmxSpacePoint(Acts::Vector3D gp, float t, int layer,
                       Acts::Vector3D cv, int id) {
            m_x = gp(0);
            m_y = gp(1);
            m_z = gp(2);
            m_t = t;
            m_r = std::sqrt(m_x*m_x + m_y*m_y);
            m_layer = layer;
            m_id = id;
            m_varianceR = cv(0);
            m_varianceZ = cv(1);
                        
        }
        */

        
        LdmxSpacePoint(const std::vector<float>& gp, float t, int layer,
                       const std::vector<float>& cv, int id) {

            m_x = gp[0];
            m_y = gp[1];
            m_z = gp[2];
            m_t = t;
            m_r = std::sqrt(m_x*m_x + m_y*m_y);
            m_layer = layer;
            m_id = id;
            m_varianceR = cv[0];
            m_varianceZ = cv[1];
            
        }
        
        
        float x() const {return m_x;}
        float y() const {return m_y;}
        float z() const {return m_z;}
        float t() const {return m_t;}
        float r() const {return m_r;}
        float varianceR() const {return m_varianceR;}
        float varianceZ() const {return m_varianceZ;}
        int   layer() const {return m_layer;}
        int   id() const {return m_id;}
        
        
    private:
        
        float m_x;
        float m_y;
        float m_z;
        float m_t;
        float m_r;
        float m_varianceR;
        float m_varianceZ;
        int   m_id;
        int   m_layer;
        
    };    
    
        

    


}
