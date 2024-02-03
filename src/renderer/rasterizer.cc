#include "rasterizer.hh"
#include "triangle.hh"

namespace Fallment{



void RasterizerLine::drawLine(int x1, int y1, int x2, int y2, const glm::vec4& color,Framebuffer& fb) {

    bool steep = false;
    // traverse by X or Y
    if(std::abs(x2 - x1) < std::abs(y2 - y1)){
        std::swap(x1, y1);
        std::swap(x2, y2);
        steep = true;
    }
    // traverse from smaller value
    if(x2 < x1){
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    int dx = x2 - x1;
    int dy = y2 - y1;

    int de2 = std::abs(dy) * 2;
    int e2 = 0;
    int y = y1;
    for(int x = x1; x <= x2; ++x){
        (steep ? fb.setPixelColor(y,x,color) : fb.setPixelColor(x,y,color));
        e2 += de2;
        if(e2 > dx){
            y += (y2 > y1 ? 1 : -1);
            e2 -= dx*2;
        }
    }
}

void RasterizerLine::drawTriangle(Triangle &tri,Shader& sh,Framebuffer& fb){
    glm::vec3 v1 = tri.avp();
    glm::vec3 v2 = tri.bvp();
    glm::vec3 v3 = tri.cvp();

    sh.fragment(*sh._fragmentIn,*sh._fragmentOut);
    auto color = sh.gl_FragColor;

    drawLine(round(v1.x), round(v1.y), round(v2.x), round(v2.y), color, fb);
    drawLine(round(v1.x), round(v1.y), round(v3.x), round(v3.y), color, fb);
    drawLine(round(v3.x), round(v3.y), round(v2.x), round(v2.y), color, fb);
}

void RasterizerFill::drawTriangle(Triangle &tri,Shader& sh,Framebuffer& fb){
    drawFill(tri,sh,fb);
}

void RasterizerFill::drawFill(Triangle &tri,Shader& sh,Framebuffer& fb) {

    glm::vec3 v1 = tri.avp();
    glm::vec3 v2 = tri.bvp();
    glm::vec3 v3 = tri.cvp();

    glm::vec2 bboxmin,bboxmax;

    bboxmin.x = std::max(0.f, std::min(v1.x, std::min(v2.x, v3.x)));
    bboxmin.y = std::max(0.f, std::min(v1.y, std::min(v2.y, v3.y)));
    bboxmax.x = std::min((float)(fb.getWidth() - 1), std::max(v1.x, std::max(v2.x, v3.x)));
    bboxmax.y = std::min((float)(fb.getHeight() - 1), std::max(v1.y, std::max(v2.y, v3.y)));

    glm::vec3 p;

    for(p.x = bboxmin.x; p.x <= bboxmax.x; p.x++){
        for(p.y = bboxmin.y; p.y <= bboxmax.y; p.y++){

            glm::vec3 bc = mth::barycentric3(v1, v2, v3, p);
            if(bc.x < 0 || bc.y < 0 || bc.z < 0) 
                continue;

            //depth test
            p.z = v1.z*bc.x + v2.z*bc.y + v3.z*bc.z;
            if(p.z<(fb.getZ(p.x,p.y)))
                continue;
            else 
                fb.setZ(p.x,p.y,p.z);

            sh.fragment(*sh._fragmentIn,*sh._fragmentOut);
            auto color = sh.gl_FragColor;

            fb.setPixelColor(p.x,p.y,color);
        }
    }
}

void RasterizerPhong::drawTriangle(Triangle &tri,Shader& sh,Framebuffer& fb){

    glm::vec3 v1 = tri.avp();
    glm::vec3 v2 = tri.bvp();
    glm::vec3 v3 = tri.cvp();

    glm::vec3 vw1 = tri.aw();
    glm::vec3 vw2 = tri.bw();
    glm::vec3 vw3 = tri.cw();

    glm::vec2 uv1 = tri.auv();
    glm::vec2 uv2 = tri.buv();
    glm::vec2 uv3 = tri.cuv();

    glm::vec3 nw1 = tri.anw();
    glm::vec3 nw2 = tri.bnw();
    glm::vec3 nw3 = tri.cnw();

    glm::vec4 c1 = tri.aco();
    glm::vec4 c2 = tri.bco();
    glm::vec4 c3 = tri.cco();


    glm::vec2 bboxmin(fb.getWidth() - 1, fb.getHeight() - 1);
    glm::vec2 bboxmax(0, 0);

    bboxmin.x = std::max(0.f, std::min(v1.x, std::min(v2.x, v3.x)));
    bboxmin.y = std::max(0.f, std::min(v1.y, std::min(v2.y, v3.y)));
    bboxmax.x = std::min((float)(fb.getWidth() - 1), std::max(v1.x, std::max(v2.x, v3.x)));
    bboxmax.y = std::min((float)(fb.getHeight() - 1), std::max(v1.y, std::max(v2.y, v3.y)));

    glm::vec3 p;
    for(p.x = bboxmin.x; p.x <= bboxmax.x; p.x++){
        for(p.y = bboxmin.y; p.y <= bboxmax.y; p.y++){

            glm::vec3 bc = mth::barycentric3(v1, v2, v3, p);
            if(bc.x < 0 || bc.y < 0 || bc.z < 0) 
                continue;

            
             //透视矫正插值 https://www.researchgate.net/publication/2893969_Perspective-Correct_Interpolation
            float z_reciprocal = bc.x*(1.0f/v1.z) + bc.y*(1.0f/v2.z) + v3.z*(1.0f/v3.z); 
            p.z = 1.0f/z_reciprocal;

            //depth test
            if(p.z<(fb.getZ(p.x,p.y)))
                continue;
            else 
                fb.setZ(p.x,p.y,p.z);

           
            glm::vec3 vw = mth::interpolate(vw1,vw2,vw3,bc,v1.z,v2.z,v3.z,z_reciprocal);
            glm::vec2 uv = mth::interpolate(uv1,uv2,uv3,bc,v1.z,v2.z,v3.z,z_reciprocal);
            glm::vec3 nw = mth::interpolate(nw1,nw2,nw3,bc,v1.z,v2.z,v3.z,z_reciprocal);
            glm::vec4 c  = mth::interpolate(c1,c2,c3,bc,v1.z,v2.z,v3.z,z_reciprocal);
               
            auto in = static_cast<FragmentInDataPhong>(*sh._fragmentIn);

            in.worldPos = vw;
            in.uv = uv;
            in.normal = nw;
            in.color = c;

            sh.fragment(*sh._fragmentIn,*sh._fragmentOut);
            fb.setPixelColor(p.x,p.y,sh.gl_FragColor);
        }
    }
    
}

RasterizerPhong::RasterizerPhong(const Rasterizer& r){

}

};