//
//  turbulentCircleEmitter.cpp
//  Particles
//
//  Created by Aaron Damashek on 5/28/13.
//
//

#include "turbulentCircleEmitter.h"

turbulentCircleEmitter::turbulentCircleEmitter(float r, particle **pool, int emitter_id, vector3 pos, vector3 dir, vector3 dirVar, float speed, float speedVar, int totalParticles, int emitsPerFrame, int emitVar, int life, int lifeVar, vector3 force) : circleEmitter(r, pool, emitter_id, pos, dir, dirVar, speed, speedVar, totalParticles, emitsPerFrame, emitVar, life, lifeVar, force){
    
}

void turbulentCircleEmitter::display(){
    if(!displaying) return;
    for(int newP = 0; newP < (e->emitsPerFrame + e->emitVar*randDist()); newP++){
        addParticle();
    }
    glEnable(GL_POINT_SMOOTH);
    glEnable( GL_TEXTURE_2D );
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
    
    glBindTexture(GL_TEXTURE_2D, 0);
    //sf::Image::Bind(); //or glBindTexture(id);
    
    glEnable(GL_POINT_SPRITE);
    glDepthMask(GL_FALSE);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    // File locations
    std::string vertexShader = "/Users/aarondamashek/Documents/Stanford Work/Spring 2013/CS 248/ParticleSystem3/Particles/kernels/default.vert";
    std::string fragmentShader = "/Users/aarondamashek/Documents/Stanford Work/Spring 2013/CS 248/ParticleSystem3/Particles/kernels/wind.frag";
    //std::string fragmentShader = "/Users/aarondamashek/Documents/Stanford Work/Spring 2013/CS 248/ParticleSystem3/ProgrammableShading/kernels/wind.frag";
    std::string windPic = "/Users/aarondamashek/Documents/Stanford Work/Spring 2013/CS 248/ParticleSystem3/Particles/wind.png";
    
    STImage   *windImg;
    STTexture *windTex;
    
    STShaderProgram *shader;
    
    windImg = new STImage(windPic);
    windTex = new STTexture(windImg);
    
    shader = new STShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);
    
    // Texture 1: fire
    glActiveTexture(GL_TEXTURE0);
    windTex->Bind();
    
    // Bind the textures we've loaded into openGl to
    // the variable names we specify in the fragment
    // shader.
    shader->SetTexture("windTex", 0);
    
    // Invoke the shader.  Now OpenGL will call our
    // shader programs on anything we draw.
    shader->Bind();
    shader->SetUniform("pointRadius", 7.0f);
    shader->SetUniform("point_size", 7.0f);
    
    glPointSize(5);
    glBegin(GL_POINTS);
    particle *curr = e->particleList;
    while(curr){
        glVertex3f(curr->pos.x, curr->pos.y, curr->pos.z);
        curr = curr->next;
    }
    glEnd();
    
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDisable(GL_POINT_SMOOTH);
    
    shader->UnBind();
    
    glActiveTexture(GL_TEXTURE0);
    windTex->UnBind();
    delete windImg;
    delete windTex;
    delete shader;
}

bool turbulentCircleEmitter::addParticle(){
    particle *newParticle;
    float speed;
    //Particle pool exists and max num particles not exceeded
    if(e != NULL && *managerParticleList != NULL && e->particleCount < e->totalParticles && emitting){
        newParticle = *managerParticleList;
        *managerParticleList = (*managerParticleList)->next;
        if(e->particleList != NULL){
            e->particleList->prev = newParticle;
        }
        newParticle->next = e->particleList;
        newParticle->prev = NULL;
        e->particleList = newParticle;
        
        float angle = randomAngle();
        float radScalar = randDist();
        newParticle->rand = radScalar;
        newParticle->radius = radius * radScalar;
        STVector3 point = STVector3(e->pos.x + radius*radScalar*cosf(angle), e->pos.y, e->pos.z + radius*radScalar*sinf(angle));
        STVector3 straightUp = STVector3(0,1,0);
        STVector3 circleDir = STVector3(e->dir.x, e->dir.y, e->dir.z);
        
        
        STVector3 a  = STVector3::Cross(straightUp, circleDir);
        float w = sqrt(powf(straightUp.Length(), 2) * powf(circleDir.Length(), 2)) + STVector3::Dot(straightUp, circleDir);
        Quaternion rotateCircle = Quaternion(w, a.x, a.y, a.z);
        rotateCircle.Normalize();
        
        STVector3 rotatedPoint = rotateCircle.rotate(point, rotateCircle);
        newParticle->pos.x = rotatedPoint.x;
        newParticle->pos.y = rotatedPoint.y;
        newParticle->pos.z = rotatedPoint.z;
        
        /*
         newParticle->pos.x = e->pos.x + radius*sinf(angle);
         newParticle->pos.y = e->pos.y;
         newParticle->pos.z = e->pos.z + radius*cosf(angle);
         */
        
        newParticle->prevPos.x = 0;
        newParticle->prevPos.y = 0;
        newParticle->prevPos.z = 0;
        
        newParticle->dir = e->dir;
        speed = 2*e->speed + e->speed * randDist()/2;
        //speed = e->speed;
        newParticle->dir.x *= speed;
        newParticle->dir.y *= speed;
        newParticle->dir.z *= speed;
        
        //newParticle->life = e->life + (int)((float)e->lifeVar * randDist());
        newParticle->life = e->life;
        newParticle->side = randDist();
        e->particleCount++;
        return true;
    }
    return false;
}

bool turbulentCircleEmitter::updateParticle(particle *p){
    if(p != NULL && p->life > 0){
        p->prevPos.x = p->pos.x;
        p->prevPos.y = p->pos.y;
        p->prevPos.z = p->pos.z;
        
        //p->dir = p->dir*(fmax((p->life),e->life/1.1)/(float)e->life);
        

        
        //p->dir.x += e->force.x*cosf(p->pos.y)*p->side;
        //p->dir.y += e->force.y*cosf(p->pos.y)*p->side;
        //p->dir.z += e->force.z*cosf(p->pos.y)*p->side;
        
        //p->dir.y += p->pos.z * p->pos.x / 20 * randDist();
        //p->dir.z += p->pos.y * p->pos.x / 20 * randDist();
        
        //p->pos.y = p->radius * cosf((p->pos.x*2 + p->rand)/e->speed) * 1.5;
        //p->pos.z = p->radius * sinf((p->pos.x*2 + p->rand)/e->speed);
        
        //p->pos.y = cosf((p->pos.x)*e->life/e->lifeVar)*this->radius*p->rand;
        //p->pos.z = p->radius * sinf((p->pos.x)/e->speed);
        
        //p->pos.y = cosf((p->pos.x)*e->life/e->lifeVar)*this->radius*p->rand;
        //p->pos.z = p->radius * sinf((p->pos.x)*e->life/e->lifeVar)*this->radius*p->rand;
        
        p->pos.y = cosf((p->pos.x)*e->life/e->lifeVar)*p->radius*p->rand;
        p->pos.z = p->radius * sinf((p->pos.x)*e->life/e->lifeVar)*p->radius*p->rand;
        
        p->pos.x += p->dir.x;
        //p->pos.y += e->dirVar.y;
        //p->pos.z += e->dirVar.z;
        
        p->life--;
        return true;
    }else if(p != NULL && p->life == 0){
        if(p->prev != NULL){
            p->prev->next = p->next;
        }else{
            e->particleList = p->next;
        }
        if(p->next != NULL){
            p->next->prev = p->prev;
        }
        p->next = *managerParticleList;
        p->prev = NULL;
        *managerParticleList = p;
        e->particleCount--;
    }
    
    return false;
}