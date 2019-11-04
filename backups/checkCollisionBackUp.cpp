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
					PTwo->canMoveDown = false;
					POne->canMoveUp = false;
					PTwo->prevPos = PTwo->pos;
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
				else if(POne->jumping == true && POne->pos.y+POne->getRad()<=PTwo->pos.y-PTwo->getRad()){
					POne->canMoveDown = false;
					PTwo->canMoveUp = false;
					POne->prevPos = POne->pos;
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
		else{
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
					if(getPlayerDist()<0.90f*(PTwo->getRad()+POne->getRad())){
						float slope = (PTwo->pos.y-POne->pos.y)/(PTwo->pos.x-POne->pos.x);
						float angle = atan(slope);
						float xDist = cos(angle)*(PTwo->getRad()+POne->getRad());
						float yDist = sin(angle)*(PTwo->getRad()+POne->getRad());
						if(slope<0.0f){
							yDist*=-1.0f;
							xDist*=-1.0f;
						}
						PTwo->pos.x = POne->pos.x + xDist;
						PTwo->pos.y = POne->pos.y + yDist;
					}
				}
				else if(POne->jumping == true){
					PTwo->canMoveDown = true;
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
