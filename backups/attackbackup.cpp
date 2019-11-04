struct attack{
		string tag;
		float damage,frames,delay,bonusDamage,remainingFrames,rad,width,height;
		bool isProjectile, isCircle, isRectangle;
		Vec2f pos;
		figure f;
		std::vector<special>specials; //critical, stun, knockback, etc
		float buttonReqs[]; //1->fast tap (duration), 2->button delay (delay time), 3->from prior attack(which attack), 4 -> hold (duration)etc
	};
	
attack a;
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