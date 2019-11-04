#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"

//for images
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace std;

#define EPS 0.00001
#define pRad 20.0
#define pRadLimit 30.0 

class ComboSequenceDetector {
	const double duration;
	const std::vector<uint8_t> sequence;
	const bool autoreset;
	
	uint8_t currentState;
	double currentDuration;
	size_t sequencePosition;
	
	uint8_t currentTargetState() const {
		if ( sequencePosition >= sequence.size() )
			return 0;
		return sequence[sequencePosition];
	}
	uint8_t previousTargetState() const {
		if ( sequencePosition >= sequence.size() || sequencePosition == 0 )
			return 0;
		return sequence[sequencePosition - 1];
	}
public:
	ComboSequenceDetector(const std::vector<uint8_t> &sequence, double duration = 1.0, bool autoreset = true) : duration(duration), sequence(sequence), currentState(0), currentDuration(0), sequencePosition(0), autoreset(autoreset) {}
	
	void keyUp(uint8_t code) {
		currentState &= ~code;
	}
	
	void keyDown(uint8_t code) {
		currentState |= code;
	}
	
	void reset() {
		currentDuration = 0;
		sequencePosition = 0;
	}
	void update(double dt) {
		if ( *this ) {
			if ( autoreset )
				reset();
			else
				return;
		}
		
		currentDuration += dt;
		if ( currentDuration > duration ) {
			reset();
		}
		
		if ( *this ) return;
		
		if ( currentState ==  currentTargetState() ) {
			++sequencePosition;
			return;
		}
		
		auto stateMask = currentTargetState() | previousTargetState();
		if ( (stateMask | currentState) ^ stateMask ) {
			reset();
			if ( currentState == currentTargetState() )
				++sequencePosition;
		} else {
			//indeterminate
		}
	}
	
	operator bool() const {
		return sequencePosition >= sequence.size();
	}
};

class EightWayApp : public AppBasic {
  public:
	float WIND_H, WIND_W; 
	//structs
	struct line{
		Vec2f start, end;
	};
	struct rectangle{
		Vec2f ur, ll;
	};
	struct circle{
		Vec2f center;
		float rad;
	};
	struct powerUp{
		int id;
		rectangle box;
		void setPowerUp(int tid, Vec2f ur, Vec2f ll){
			id = tid;
			box.ur = ur;
			box.ll = ll;
		}
	};
	struct stage{
		vector<rectangle>obstacles;
		void setStage(int rectCount, vector<Vec2f>positions){
			for(int i=0;i<positions.size()-1;i++){
				rectangle r;
				r.ur=positions[i];
				r.ll=positions[i+1];
				obstacles.push_back(r);
			}
		}
	};
	struct figure{
		char tag;
		Vec2f ul,lr,center,a,b,c; //rectangle, circle, triangle
		line extentI, extentJ, sideOne, sideTwo, sideThree, sideFour; //for rectangles and triangles
		float radius, angle, halfwidth, halfheight, perimeter, circumference, area, semiperemeter;
	};
	struct special{
		string tag,effect;
		float chance, bonusDamage, duration;
	};
	struct attack{
		string tag;
		float damage, frames, delay, bonusDamage, remainingFrames, rad, width, length;
		bool isProjectile, isCircle, isRectangle;
		Vec2f pos;
		figure f;
		std::vector<special>specials; //critical, stun, knockback, etc
		float buttonReqs[]; //1->fast tap (duration), 2->button delay (delay time), 3->from prior attack(which attack), 4 -> hold (duration)etc
	};
	struct projectile{
		Vec2f velocity, pos, initPos, direction;
		bool isCircle, isRect;
		float length, width, rad, damage, bonusDamage, angle;
	};
	struct Player{
		//spritesheet variable
		bool moving, vertical, jumping, upKey, downKey, jumpKey, leftKey, rightKey, canMoveUp, canMoveDown, canMoveRight, canMoveLeft, canJump;
		bool falling, displaced;
		int PlayerNum,direction;
		float radius,angle,startLife,currentLife,dispVel;
		std::vector<attack>attackQueue;
		std::vector<attack>standardAtks;
		Vec2f pos,velocity, prevPos;
		figure f;
		std::vector<ComboSequenceDetector*> combos;
		void jump();
		void move();
		void attack();
		void addCombo(const std::vector<uint8_t> &sequence, double duration = 1.0, bool autoreset = true){
			ComboSequenceDetector combo(sequence, duration, autoreset);
			combos.push_back(&combo);
		}
		void setCombos(){
			if(PlayerNum == 1){
				std::vector<uint8_t>comboOne;
				comboOne.push_back(KeyEvent::KEY_j);
				addCombo(comboOne);
				//make character 1 combos here
			}
			else if(PlayerNum == 2){
				//make character 1 combos here
			}
			else if(PlayerNum == 3){
				//make character 1 combos here
			}
			else if(PlayerNum == 4){
				//make character 1 combos here
			}
		}
		void set(){
			radius = pRad;
			velocity.x = 8.0f;
			velocity.y = 8.0f;
			dispVel = 1.0f;
			moving = upKey = downKey = jumpKey = leftKey = rightKey = vertical = jumping = false;
			canJump = canMoveUp = canMoveDown = canMoveRight = canMoveLeft = true;			
		}
		void setPosition(Vec2f p){
			pos = p;
		}
		Vec2f getPos(){
			return pos;
		}
		float getRad(){
			return radius;
		}
	};

	//methods
	void setup();
	void setAttacks(Player *p);
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	void mouseDown( MouseEvent event );
	//void mouseUp( MouseEvent event );
	void checkCollision();
	void update();
	void draw();
	float fRand(float fMin, float fMax);
	float getPlayerDist();

	//Players
	Player *POne;
	Player *PTwo;
	
	//vector of projectiles
	vector<projectile>projectiles;
	
	//stage
	stage s;
};

void EightWayApp::prepareSettings( Settings *settings ){
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
}

float EightWayApp::fRand(float fMin, float fMax){
    float f = (float)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}
void EightWayApp::setup()
{
	WIND_H = this->getWindowHeight();
	WIND_W = this->getWindowWidth();
	//---------------------------------//
	//Player Details
	POne = new Player();
	PTwo = new Player();
	POne->set();
	PTwo->set();
	POne->startLife = 100.0f;
	POne->currentLife = POne->startLife;
	PTwo->startLife = 100.0f;
	PTwo->currentLife = PTwo->startLife;
	POne->setPosition(Vec2f(WIND_W*0.5-(2*POne->getRad()),WIND_H*0.5));
	PTwo->setPosition(Vec2f(WIND_W*0.5+(2*PTwo->getRad()),WIND_H*0.5));
	//---------------------------------//
	
	
	//---------------------------------//
	//Stage Details
	vector<Vec2f>rectPos;
	int size = (int)(fRand(1,10));
	size = 10;
	float maxX = WIND_W;
	float minX = 0.0f;
	float maxY = WIND_H - 100.0f;
	float minY = 0.0f;
	for(int i=0;i<size;i++){
		Vec2f ur,ll;
		ur.x = fRand(minX,maxX);
		ur.y = fRand(minY,maxY);
		ll.x = ur.x+(fRand(5.0f,20.0f));
		ll.y = ur.y+(fRand(5.0f,30.0f));
		ur.x = ((float)rand()/(float)RAND_MAX)*(maxX-minX)+minX;
		ur.y = ((float)rand()/(float)RAND_MAX)*(maxY-minY)+minY;
		ll.x = ur.x + ((float)rand()/(float)RAND_MAX)*25.0f + 5.0f;
		ll.y = ur.y + ((float)rand()/(float)RAND_MAX)*25.0f + 5.0f;
		rectPos.push_back(ur);
		rectPos.push_back(ll);
	}
	//s.setStage(size, rectPos);
	for(int i=0;i<rectPos.size()-1;i++){
		rectangle r;
		r.ur=rectPos[i];
		r.ll=rectPos[i+1];
		s.obstacles.push_back(r);
	}
	//---------------------------------//
}


void EightWayApp::setAttacks(Player *p){
	attack a;
	//a attributes
		a.tag = "punch";
		a.damage = 5.0f;
		a.frames = 15.0f;
		a.delay = 5.0f;
		a.bonusDamage = 0.0f;
		a.remainingFrames = a.frames;
		a.width = p->getRad()*0.25;
		a.length = p->getRad()*0.5;
		a.pos.x = p->getRad()*cos(p->angle);
		a.pos.y = p->getRad()*sin(p->angle);
		a.isCircle = false;
		a.isRectangle = true;
		a.isProjectile = false;

	attack b;
		//b attributes
		b.tag = "projectile";
		b.damage = 4.0f;
		b.frames = 10.0f;
		b.delay = 10.0f;
		b.bonusDamage = 2.0f;
		b.remainingFrames = b.frames;
		b.rad = p->getRad()*0.3;
		b.pos.x = p->getRad()*cos(p->angle);
		b.pos.y = p->getRad()*sin(p->angle);
		b.isCircle = true;
		b.isRectangle = false;
		b.isProjectile = true;	

	attack c;
		//c attributes
		c.tag = "weapon";
		c.damage = 5.0f;
		c.frames = 20.0f;
		c.delay = 5.0f;
		c.bonusDamage = 3.0f;
		c.remainingFrames = c.frames;
		c.width = p->getRad()*0.25;
		c.length = p->getRad()*0.75;
		c.pos.x = p->getRad()*cos(p->angle);
		c.pos.y = p->getRad()*sin(p->angle);
		c.isCircle = false;
		c.isRectangle = true;
		c.isProjectile = false;	
	attack d;
	p->standardAtks.push_back(a);
	p->standardAtks.push_back(b);
	p->standardAtks.push_back(c);
	p->standardAtks.push_back(d);
}

void EightWayApp::keyDown( KeyEvent event )
{
	int code = event.getCode();
	//WASD = player one movement
	//YGHJ = player one attack -> for all presses of attack keys, update all player's CSDs
	//space = player on jump
	if (code==KeyEvent::KEY_a && POne->rightKey == false) {
		POne->leftKey = true;
		POne->angle = 225.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_d && POne->leftKey == false) {
		POne->rightKey = true;
		POne->angle = 45.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_w && POne->downKey == false) {
		POne->upKey = true;
		POne->angle = 135.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_s && POne->upKey == false) {
		POne->downKey = true;
		POne->angle = 315.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_SPACE) {
		POne->jumpKey = true;
	}
	if (code==KeyEvent::KEY_y || code==KeyEvent::KEY_g || code==KeyEvent::KEY_h || code==KeyEvent::KEY_j){
		for(int i=0;i<POne->combos.size();i++){
			POne->combos[i]->keyDown(code);
		}
	}
	if (code==KeyEvent::KEY_LEFT && PTwo->rightKey == false) {
		PTwo->leftKey = true;
		POne->angle = 225.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_RIGHT && PTwo->leftKey == false) {
		PTwo->rightKey = true;
		POne->angle = 45.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_UP && PTwo->downKey == false) {
		PTwo->upKey = true;
		POne->angle = 135.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_DOWN && PTwo->upKey == false) {
		PTwo->downKey = true;
		POne->angle = 315.0f*M_PI/180.0f;
	}

	if (code==KeyEvent::KEY_PAGEDOWN) {
		PTwo->jumpKey = true;
	}
}

void EightWayApp::keyUp( KeyEvent event ){
	int code = event.getCode();
	if (code==KeyEvent::KEY_a) {
		POne->leftKey = false;
	}

	if (code==KeyEvent::KEY_d) {
		POne->rightKey = false;
	}

	if (code==KeyEvent::KEY_SPACE) {
		POne->jumpKey = false;
	}

	if (code==KeyEvent::KEY_w) {
		POne->upKey = false;
	}

	if (code==KeyEvent::KEY_s) {
		POne->downKey = false;
	}
	if (code==KeyEvent::KEY_LEFT) {
		PTwo->leftKey = false;
	}

	if (code==KeyEvent::KEY_RIGHT) {
		PTwo->rightKey = false;
	}

	if (code==KeyEvent::KEY_UP) {
		PTwo->upKey = false;
	}

	if (code==KeyEvent::KEY_DOWN) {
		PTwo->downKey = false;
	}

	if (code==KeyEvent::KEY_PAGEDOWN) {
		PTwo->jumpKey = false;
	}
}

void EightWayApp::mouseDown( MouseEvent event )
{
}
float EightWayApp::getPlayerDist(){
	return sqrt((POne->pos.x-PTwo->pos.x)*(POne->pos.x-PTwo->pos.x) + (POne->pos.y-PTwo->pos.y)*(POne->pos.y-PTwo->pos.y));
}
void EightWayApp::checkCollision(){
	if(POne->displaced == true){
	}
	else if(PTwo->displaced == true){
	}
	if(getPlayerDist() <= POne->getRad() + PTwo->getRad()){
		if(POne->pos.x == PTwo->pos.x){
			if(POne->pos.y >= PTwo->pos.y){
				if(PTwo->jumping == false){
					POne->canMoveUp = false;
					PTwo->canMoveDown = false;
					if(POne->moving == true && PTwo->moving == true){
						float intPoint = PTwo->pos.y - (getPlayerDist()*0.5);
						POne->pos.y = intPoint - POne->getRad();
						PTwo->pos.y = intPoint + PTwo->getRad();
					}
					else if(POne->moving == true)
						POne->pos.y = PTwo->pos.y-(POne->getRad()+PTwo->getRad());
					else if(PTwo->moving == true)
						PTwo->pos.y = POne->pos.y+(PTwo->getRad()+POne->getRad());
				}
				else if(PTwo->jumping == true){
					POne->canMoveDown = true;
				}
			}
			else if(POne->pos.y <= PTwo->pos.y){
				if(POne->jumping == false){
					POne->canMoveDown = false;
					PTwo->canMoveUp = false;
					if(POne->moving == true && PTwo->moving == true){
						float intPoint = PTwo->pos.y + (getPlayerDist()*0.5);
						POne->pos.y = intPoint + POne->getRad();
						PTwo->pos.y = intPoint - PTwo->getRad();
					}
					else if(POne->moving == true)
						POne->pos.y = PTwo->pos.y+(POne->getRad()+PTwo->getRad());
					else if(PTwo->moving == true)
						PTwo->pos.y = POne->pos.y-(PTwo->getRad()+POne->getRad());
				}
				else if(POne->jumping == true){
					PTwo->canMoveDown = true;
				}
			}
		}
		else if(POne->pos.y == PTwo->pos.y){
			if(POne->pos.x <= PTwo->pos.x){
				POne->canMoveRight = false;
				PTwo->canMoveLeft = false;
				if(POne->moving == true && PTwo->moving == true){
					float intPoint = PTwo->pos.x - (getPlayerDist()*0.5);
					POne->pos.x = intPoint - POne->getRad();
					PTwo->pos.x = intPoint + PTwo->getRad();
				}
				else if(POne->moving == true)
					POne->pos.x = PTwo->pos.x-(POne->getRad()+PTwo->getRad());
				else if(PTwo->moving == true)
					PTwo->pos.x = POne->pos.x+(PTwo->getRad()+POne->getRad()); 
			}
			else if(POne->pos.x >= PTwo->pos.x){
				POne->canMoveLeft = false;
				PTwo->canMoveRight = false;
				if(POne->moving == true && PTwo->moving == true){
					float intPoint = PTwo->pos.x + (getPlayerDist()*0.5);
					POne->pos.x = intPoint + POne->getRad();
					PTwo->pos.x = intPoint - PTwo->getRad();
				}
				else if(POne->moving == true)
					POne->pos.x = PTwo->pos.x+(POne->getRad()+PTwo->getRad());
				else if(PTwo->moving == true)
					PTwo->pos.x = POne->pos.x-(PTwo->getRad()+POne->getRad());
			}
			if(PTwo->jumping == false){
				POne->canMoveUp = false;
				PTwo->canMoveDown = false;
				if(getPlayerDist()<0.9*(PTwo->getRad()+POne->getRad())){
					float slope = (PTwo->pos.y-POne->pos.y)/(PTwo->pos.x-POne->pos.x);
					float angle = atan(slope);
					float xDist = cos(angle)*(PTwo->getRad()+POne->getRad());
					float yDist = sin(angle)*(PTwo->getRad()+POne->getRad());
					if(slope>0.0f){
						yDist*=-1.0f;
						//xDist*=-1.0f;
					}
					if(POne->pos.x>=PTwo->pos.x)
						POne->pos.x = PTwo->pos.x + xDist;
					else if(POne->pos.x<PTwo->pos.x)
						POne->pos.x = PTwo->pos.x - xDist;
					POne->pos.y = PTwo->pos.y + yDist;
				}
			}
			else{
				PTwo->canMoveDown = true;
			}
			if(POne->jumping==false){
				PTwo->canMoveUp = false;
				POne->canMoveDown = false;
				if(getPlayerDist()<0.9*(PTwo->getRad()+POne->getRad())){
					float slope = (PTwo->pos.y-POne->pos.y)/(PTwo->pos.x-POne->pos.x);
					float angle = atan(slope);
					float xDist = cos(angle)*(PTwo->getRad()+POne->getRad());
					float yDist = sin(angle)*(PTwo->getRad()+POne->getRad());
					if(slope>0.0f){
						yDist*=-1.0f;
						//xDist*=-1.0f;
					}
					if(PTwo->pos.x >= POne->pos.x)
						PTwo->pos.x = POne->pos.x + xDist;
					else if(PTwo->pos.x < POne->pos.x)
						PTwo->pos.x = POne->pos.x - xDist;
					PTwo->pos.y = POne->pos.y + yDist;
				}
			}
			else{
				PTwo->canMoveDown = true;
			}
		}
		else{
			if(POne->pos.y > PTwo->pos.y){
				if(PTwo->jumping == false){
					POne->canMoveUp = false;
					PTwo->canMoveDown = false;
					float slope = (POne->pos.y - PTwo->pos.y)/(POne->pos.x - PTwo->pos.x);
					float angle = atan(slope);
					float yDist = getPlayerDist()*sin(angle);
					if(slope<0.0f)
						yDist*=-1.0f;
					if(POne->vertical == true && PTwo->vertical == true){
						float topDist = PTwo->getRad()*sin(angle);
						float intPoint = PTwo->pos.y + topDist;
						POne->pos.y = intPoint + (yDist-topDist);
						PTwo->pos.y = intPoint - topDist;
					}
					else if(POne->vertical == true)
						POne->pos.y = PTwo->pos.y+yDist;
					else if(PTwo->vertical == true)
						PTwo->pos.y = POne->pos.y-yDist;
				}
	
				else{
					POne->canMoveDown = true;
				}
				if(POne->jumping==false && PTwo->jumping==false){
					PTwo->canMoveUp = false;
					POne->canMoveDown = false;
					if(getPlayerDist()<0.9*(PTwo->getRad()+POne->getRad())){
						float slope = (PTwo->pos.y-POne->pos.y)/(PTwo->pos.x-POne->pos.x);
						float angle = atan(slope);
						float xDist = cos(angle)*(PTwo->getRad()+POne->getRad());
						float yDist = sin(angle)*(PTwo->getRad()+POne->getRad());
						if(slope>0.0f){
							yDist*=-1.0f;
							xDist*=-1.0f;
						}
						PTwo->pos.x = POne->pos.x + xDist;
						PTwo->pos.y = POne->pos.y + yDist;
					}
				}
				else{
					PTwo->canMoveDown = true;
				}
			}
			else if(POne->pos.y < PTwo->pos.y){
				if(POne->jumping == false){
					POne->canMoveDown = false;
					PTwo->canMoveUp = false;
					float slope = (POne->pos.y - PTwo->pos.y)/(POne->pos.x - PTwo->pos.x);
					float angle = atan(slope);
					float yDist = getPlayerDist()*sin(angle);
					if(slope<0.0f)
						yDist*=-1.0f;
					if(POne->vertical == true && PTwo->vertical == true){
						float topDist = POne->getRad()*sin(angle);
						float intPoint = POne->pos.y + topDist;
						POne->pos.y = intPoint - topDist;
						PTwo->pos.y = intPoint + (yDist-topDist);
					}
					else if(POne->vertical == true)
						POne->pos.y = PTwo->pos.y-yDist;
					else if(PTwo->vertical == true)
						PTwo->pos.y = POne->pos.y+yDist;
				}
				else if(POne->jumping == true){
					PTwo->canMoveDown = true;
				}
				if(PTwo->jumping==false && POne->jumping == false){
					POne->canMoveUp = false;
					PTwo->canMoveDown = false;
					if(getPlayerDist()<0.9*(PTwo->getRad()+POne->getRad())){
						float slope = (PTwo->pos.y-POne->pos.y)/(PTwo->pos.x-POne->pos.x);
						float angle = atan(slope);
						float xDist = cos(angle)*(PTwo->getRad()+POne->getRad());
						float yDist = sin(angle)*(PTwo->getRad()+POne->getRad());
						if(slope>0.0f){
							yDist*=-1.0f;
							xDist*=-1.0f;
						}
						POne->pos.x = PTwo->pos.x + xDist;
						POne->pos.y = PTwo->pos.y + yDist;
					}
				}
				else{
					POne->canMoveDown = true;
				}
			}
			if(POne->pos.x <= PTwo->pos.x){
				POne->canMoveRight = false;
				PTwo->canMoveLeft = false;
				float slope = (POne->pos.y - PTwo->pos.y)/(POne->pos.x - PTwo->pos.x);
				float angle = atan(slope);
				float xDist = getPlayerDist()*cos(angle);
				if(POne->moving == true && PTwo->moving == true){
					float intPoint = PTwo->pos.x - (xDist*0.5);
					POne->pos.x = intPoint - xDist*0.5;
					PTwo->pos.x = intPoint + xDist*0.5;
				}
				else if(POne->moving == true)
					POne->pos.x = PTwo->pos.x-xDist;
				else if(PTwo->moving == true)
					PTwo->pos.x = POne->pos.x+xDist; 
			}
			else if(POne->pos.x >= PTwo->pos.x){
				POne->canMoveLeft = false;
				PTwo->canMoveRight = false;
				float slope = (POne->pos.y - PTwo->pos.y)/(POne->pos.x - PTwo->pos.x);
				float angle = atan(slope);
				float xDist = getPlayerDist()*cos(angle);;
				if(POne->moving == true && PTwo->moving == true){
					float intPoint = PTwo->pos.x + (xDist*0.5);
					POne->pos.x = intPoint + xDist*0.5;
					PTwo->pos.x = intPoint - xDist*0.5;
				}
				else if(POne->moving == true)
					POne->pos.x = PTwo->pos.x+xDist;
				else if(PTwo->moving == true)
					PTwo->pos.x = POne->pos.x-xDist; 
			}
		}
	}
	else{
		POne->canMoveDown = POne->canMoveLeft = POne->canMoveRight = POne->canMoveUp = true;
		PTwo->canMoveDown = PTwo->canMoveLeft = PTwo->canMoveRight = PTwo->canMoveUp = true;
	}
	if(POne->pos.x - POne->getRad() <= 0){
		POne->canMoveLeft = false;
		POne->pos.x = POne->getRad();
	}
	if(POne->pos.x + POne->getRad() >= WIND_W){
		POne->canMoveRight = false;
		POne->pos.x = WIND_W-POne->getRad();
	}
	if(POne->pos.y - POne->getRad() <= 0){
		POne->canMoveUp = false;
		POne->pos.y = POne->getRad();
	}
	if(POne->pos.y + POne->getRad() >= (WIND_H-80)){
		POne->canMoveDown = false;
		POne->pos.y = (WIND_H-80)-POne->getRad();
	}
	if(PTwo->pos.x - PTwo->getRad() <= 0){
		PTwo->canMoveLeft = false;
		PTwo->pos.x = PTwo->getRad();
	}
	if(PTwo->pos.x + PTwo->getRad() >= WIND_W){
		PTwo->canMoveRight = false;
		PTwo->pos.x = WIND_W-PTwo->getRad();
	}
	if(PTwo->pos.y - PTwo->getRad() <= 0){
		PTwo->canMoveUp = false;
		PTwo->pos.y = PTwo->getRad();
	}
	if(PTwo->pos.y + PTwo->getRad() >= (WIND_H-80)){
		PTwo->canMoveDown = false;
		PTwo->pos.y = (WIND_H-80)-PTwo->getRad();
	}
}

void EightWayApp::update()
{
	checkCollision();
	if(POne->canJump== true && POne->vertical==false ){
		if( POne->jumping == false && POne->jumpKey == true) {
			POne->velocity.y = 12;
			if(POne->moving == true)
				POne->velocity.y = 8;
			POne->jumping = true;
		}
		if( POne->jumping == true && POne->vertical==false ) {
			if ( (POne->pos.y - POne->velocity.y) <= (POne->prevPos.y) ) {
				POne->pos.y -= POne->velocity.y;
				POne->velocity.y -= 1;
			} else {
				POne->jumping = false;
				POne->prevPos = POne->pos;
			}
		}
	}
	//at the same time (smooth movement)
	if(POne->moving == false && POne->vertical == false && POne->leftKey == true && POne->upKey == true && POne->canMoveLeft == true && POne->canMoveUp == true) {
		POne->moving = true;
		//POne->vertical = true;
		POne->velocity.x = -6;
		//POne->velocity.y = -6;
		//POne->prevPos = POne->pos;
	}else if(POne->moving == false && POne->vertical == false && POne->leftKey == true && POne->downKey == true && POne->canMoveLeft == true && POne->canMoveDown == true) {
		POne->moving = true;
		//POne->vertical = true;
		POne->velocity.x = -6;
		//POne->velocity.y = 6;
		//POne->prevPos = POne->pos;
	} else if(POne->moving == false && POne->vertical == false && POne->rightKey == true && POne->upKey == true && POne->canMoveRight == true && POne->canMoveUp == true) {
		POne->moving = true;
		//POne->vertical = true;
		POne->velocity.x = 6;
		//POne->velocity.y = -6;
		//POne->prevPos = POne->pos;
	} else if(POne->moving == false && POne->vertical == false && POne->rightKey == true && POne->downKey == true && POne->canMoveRight == true && POne->canMoveDown == true) {
		POne->moving = true;
		//POne->vertical = true;
		POne->velocity.x = 6;
		//POne->velocity.y = 6;
		//POne->prevPos = POne->pos;
	}else {
		POne->moving = false;
		POne->vertical = false;
	}
	//left or right
	if(POne->vertical == false){
		if(POne->moving == false && POne->leftKey == true && POne->canMoveLeft == true) {
			POne->moving = true;
			POne->velocity.x = -6;
		} else if(POne->moving == false && POne->rightKey == true && POne->canMoveRight == true) {
			POne->moving = true;
			POne->velocity.x = 6;
		} else {
			POne->moving = false;
		}
	}
	//up or down
	if(POne->moving == false && POne->jumping == false){
		if(POne->vertical == false && POne->upKey == true && POne->canMoveUp == true) {
			POne->vertical = true;
			POne->velocity.y = -6;
			POne->prevPos = POne->pos;
		} else if(POne->vertical == false && POne->downKey == true && POne->canMoveDown == true) {
			POne->vertical = true;
			POne->velocity.y = 6;
			POne->prevPos = POne->pos;
		} else {
			POne->vertical = false;
		}
	}
	if(POne->moving == true) {
		POne->pos.x = POne->pos.x + POne->velocity.x;
	}
	if(POne->vertical == true) {
		POne->pos.y = POne->pos.y + POne->velocity.y;
		POne->prevPos = POne->pos;
	}
	//player two
	if(PTwo->canJump == true && PTwo->vertical==false ){
		if( PTwo->jumping == false && PTwo->jumpKey == true) {
			PTwo->velocity.y = 12;
			if(PTwo->moving == true)
				PTwo->velocity.y = 8;
			PTwo->jumping = true;
		}
		if( PTwo->jumping == true && PTwo->vertical==false ) {
			if ( (PTwo->pos.y - PTwo->velocity.y) <= (PTwo->prevPos.y) ) {
				PTwo->pos.y -= PTwo->velocity.y;
				PTwo->velocity.y -= 1;
			} else {
				PTwo->jumping = false;
				PTwo->prevPos = PTwo->pos;
			}
		}
	}
	//at the same time (smooth movement)/*
	if(PTwo->moving == false && PTwo->vertical == false && PTwo->leftKey == true && PTwo->upKey == true && PTwo->canMoveLeft == true && PTwo->canMoveUp == true) {
		PTwo->moving = true;
		//PTwo->vertical = true;
		PTwo->velocity.x = -6;
		//PTwo->velocity.y = -6;
		//PTwo->prevPos = PTwo->pos;
	}else if(PTwo->moving == false && PTwo->vertical == false && PTwo->leftKey == true && PTwo->downKey == true && PTwo->canMoveLeft == true && PTwo->canMoveDown == true) {
		PTwo->moving = true;
		//PTwo->vertical = true;
		PTwo->velocity.x = -6;
		//PTwo->velocity.y = 6;
		//PTwo->prevPos = PTwo->pos;
	} else if(PTwo->moving == false && PTwo->vertical == false && PTwo->rightKey == true && PTwo->upKey == true && PTwo->canMoveRight == true && PTwo->canMoveUp == true) {
		PTwo->moving = true;
		//PTwo->vertical = true;
		PTwo->velocity.x = 6;
		//PTwo->velocity.y = -6;
		//PTwo->prevPos = PTwo->pos;
	} else if(PTwo->moving == false && PTwo->vertical == false && PTwo->rightKey == true && PTwo->downKey == true && PTwo->canMoveRight == true && PTwo->canMoveDown == true) {
		PTwo->moving = true;
		//PTwo->vertical = true;
		PTwo->velocity.x = 6;
		//PTwo->velocity.y = 6;
		//PTwo->prevPos = PTwo->pos;
	}else {
		PTwo->moving = false;
		PTwo->vertical = false;
	}
	//left or right
	if(PTwo->vertical == false){
		if(PTwo->moving == false && PTwo->leftKey == true && PTwo->canMoveLeft == true) {
			PTwo->moving = true;
			PTwo->velocity.x = -6;
		} else if(PTwo->moving == false && PTwo->rightKey == true && PTwo->canMoveRight == true) {
			PTwo->moving = true;
			PTwo->velocity.x = 6;
		} else {
			PTwo->moving = false;
		}
	}
	//up or down
	if(PTwo->moving == false && PTwo->jumping == false){
		if(PTwo->vertical == false && PTwo->upKey == true && PTwo->canMoveUp == true) {
			PTwo->vertical = true;
			PTwo->velocity.y = -6;
			PTwo->prevPos = PTwo->pos;
		} else if(PTwo->vertical == false && PTwo->downKey == true && PTwo->canMoveDown == true) {
			PTwo->vertical = true;
			PTwo->velocity.y = 6;
			PTwo->prevPos = PTwo->pos;
		} else {
			PTwo->vertical = false;
		}	
	}
	if(PTwo->moving == true) {
		PTwo->pos.x = PTwo->pos.x + PTwo->velocity.x;
	}
	if(PTwo->vertical == true) {
		PTwo->pos.y = PTwo->pos.y + PTwo->velocity.y;
		PTwo->prevPos = PTwo->pos;
	}
}

void EightWayApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	if(POne->jumping == true){
		if(POne->prevPos.y < PTwo->pos.y){
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
		}
		else{
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
		}
	}
	else if(PTwo->jumping == true){
		if(POne->pos.y < PTwo->prevPos.y){
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
		}
		else{
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
		}
	}
	else{
		if(POne->pos.y < PTwo->pos.y){
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
		}
		else{
			gl::color( Color( 0, 1.0f, 0 ) ); 
			gl::drawSolidCircle(PTwo->getPos(),PTwo->getRad());
			gl::color( Color( 1.0f, 0, 0 ) ); 
			gl::drawSolidCircle(POne->getPos(),POne->getRad());
		}
	}
	gl::color( Color( (100.0f/255.0f), (130.0f/255.0f), (180.0f/255.0f)) );
	gl::drawSolidRect( Rectf(0,WIND_H-80,WIND_W,WIND_H),4);
	gl::color( Color( 0.9f, 0, 0 ) );
	gl::drawSolidRect( Rectf(20,WIND_H-60,20+2*POne->startLife,WIND_H-20),4);
	gl::drawSolidRect( Rectf(WIND_W - 20,WIND_H-60,WIND_W-20-2*PTwo->startLife,WIND_H-20),4);
	gl::color( Color( 0, 0.9f, 0 ) ); 
	gl::drawSolidRect( Rectf(20,WIND_H-60,20+2*POne->currentLife,WIND_H-20),4);
	gl::drawSolidRect( Rectf(WIND_W - 20,WIND_H-60,WIND_W-20-2*PTwo->currentLife,WIND_H-20),4);
	gl::color( Color( 0.7f, 0.9f, 0 ) );
	/*
	for(int i = 0; i < s.obstacles.size(); i++){
		gl::drawSolidRect( Rectf(s.obstacles[i].ur.x,s.obstacles[i].ur.y,s.obstacles[i].ll.x,s.obstacles[i].ll.y),4);
	}
	*/
}

CINDER_APP_BASIC( EightWayApp, RendererGl )
