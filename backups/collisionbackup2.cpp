if(POne->pos.y >= PTwo->pos.y){
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
else if(POne->pos.y <= PTwo->pos.y){
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
	if(PTwo->jumping==false){
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