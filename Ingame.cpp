#include "Ingame.h"

Ingame::Ingame(){
    this->gameFont = TTF_OpenFont("visitor2.ttf", SCREEN_WIDTH/20);
    this->textCombo = NULL;
    this->textLife = NULL;
    this->textScore = NULL;
    this->Init();
	srand(10);
}

Ingame::~Ingame(){
    for(unsigned int i = 0; i < this->enemies.size(); i++){
        this->DeleteEnemy(i);
        i--;
    }
    if(this->textCombo != NULL){
        SDL_FreeSurface(this->textCombo);
        SDL_FreeSurface(this->textLife);
        SDL_FreeSurface(this->textScore);
    }
    TTF_CloseFont(this->gameFont);
}

void Ingame::DeleteEnemy(int index){
	delete this->enemies[index];
    this->enemies.erase(this->enemies.begin() + index);
}

void Ingame::Init(){
    this->SetScore(0);
    this->SetCombo(1);
    this->lifes = 4;
    
    int divided = 1;
    #if PSP
    divided = 2;
    #endif
    
    this->player.Width = 40/divided;
    this->player.Height = 40/divided;
    this->enemySpeed = 0.2f/divided;
    this->playerSpeed = 0.3f/divided;

    this->player.Position.X = SCREEN_WIDTH/2;
    this->player.Position.Y = SCREEN_HEIGHT - this->player.Height;
    
    this->direction.X = 0.f;
    this->direction.Y = 0.f;

    this->currentWave = 1;

    this->waveTimer = 1000;
    this->currentWaveTimer = 0;
    
    this->canPause = false;
    this->RefreshTextSurface(&this->textLife, this->lifes);
    for(unsigned int i = 0; i < this->enemies.size(); i++){
        this->DeleteEnemy(i);
        i--;
    }
}

void Ingame::Event(SDL_Event* ev, GAME_STATE* gs){
    if(ev->type == SDL_KEYDOWN){
        if(ev->key.keysym.sym == SDLK_RIGHT){
        	this->direction.X += this->playerSpeed;
        }else if(ev->key.keysym.sym == SDLK_LEFT){
        	this->direction.X -= this->playerSpeed;
        }else if(ev->key.keysym.sym == SDLK_ESCAPE){
            *gs = PAUSE;
        }
    }else if(ev->type == SDL_KEYUP){
        if(ev->key.keysym.sym == SDLK_RIGHT){
        	this->direction.X -= this->playerSpeed;
        }else if(ev->key.keysym.sym == SDLK_LEFT){
        	this->direction.X += this->playerSpeed;
        }
    }
}

#if PSP
void Ingame::Event(SceCtrlData* pad, SDL_Event* ev, GAME_STATE* gs){
    if(pad->Buttons != 0)
    {
        if(this->canPause && (pad->Buttons & PSP_CTRL_START || pad->Buttons & PSP_CTRL_TRIANGLE || pad->Buttons & PSP_CTRL_SELECT)){
            *gs = PAUSE;
            this->canPause = false;
        }

        if(pad->Buttons & PSP_CTRL_RIGHT){
            this->direction.X = this->playerSpeed;
        }else if(pad->Buttons & PSP_CTRL_LEFT){
            this->direction.X = -this->playerSpeed;
        }else{
        	this->direction.X = 0.f;
        }
    }else{
        this->direction.X = 0.0f;
        this->canPause = true;
    }
}
#endif

void Ingame::Update(Uint32 gameTime, GAME_STATE* gs){

	for(unsigned int i = 0; i < this->enemies.size(); i++){
		this->enemies[i]->Update(gameTime);
		
		if(this->enemies[i]->CollideWith(&this->player)){
            if(this->enemies[i]->IsColored(BLUE)){
                this->RemoveLife(gs);
            }else{
                this->SetCombo(++this->combo);
            }
            this->enemies[i]->Kill();
		}
		
		if(this->enemies[i]->IsDead() || this->IsOutOfBounds(this->enemies[i])){
            this->AddScore(this->combo*100);
			this->DeleteEnemy(i);
			i--;
		}
	}
	
	Vector2 prevPos = this->player.Position;
	this->player.Position = this->player.Position + (this->direction * gameTime);
	if(this->IsOutOfBounds(&this->player)){
		this->player.Position = prevPos;
	}
	
    this->currentWaveTimer += gameTime;
    if(this->waveTimer < 100){
        this->waveTimer = 1000;
        if(this->enemySpeed < 10.f && this->playerSpeed < 10.f){
            this->enemySpeed = this->enemySpeed * 1.3f;
            this->playerSpeed = this->playerSpeed * 1.3f;
        }
        this->currentWave++;
    }
    if(this->currentWaveTimer > this->waveTimer){
		this->GenerateWave();
        this->currentWaveTimer -= this->waveTimer;
        this->waveTimer -= 50;
    }
}

void Ingame::GenerateWave(){
	int divider = 1;
	#if PSP
	divider = 2;
	#endif

    for(int i = 0; i < this->currentWave; i++){
        Vector2 pos;
        pos.X = rand()%SCREEN_WIDTH;
        pos.Y = 1 - 60/divider;
        SDL_Color col = BLUE;
        if(rand()%10 == 1){
            col = GREEN;
        }

    	this->enemies.push_back(new Enemy(pos, Vector2(60/divider, 60/divider), Vector2(0.f, this->enemySpeed), 0, col));
    }
}

bool Ingame::IsOutOfBounds(Enemy* e){
    if(e->Position.X + e->Width < 0){
        return true;
    }else if(e->Position.X > SCREEN_WIDTH){
        return true;
    }
    if(e->Position.Y > SCREEN_HEIGHT){
        return true;
    }else if(e->Position.Y + e->Height < 0){
        return true;
    }

    return false;
}

bool Ingame::IsOutOfBounds(Rectangle* e){
    if(e->Position.X < 0){
        return true;
    }else if(e->Position.X + e->Width > SCREEN_WIDTH){
        return true;
    }
    if(e->Position.Y + e->Height > SCREEN_HEIGHT){
        return true;
    }else if(e->Position.Y < 0){
        return true;
    }

    return false;
}

void Ingame::AddScore(int score){
    this->score += score * this->combo;
    this->RefreshTextSurface(&this->textScore, this->score);
}

void Ingame::SetScore(int score){
    this->score = score;
    this->RefreshTextSurface(&this->textScore, this->score);
}

void Ingame::SetCombo(int combo){
    this->combo = combo;
    this->RefreshTextSurface(&this->textCombo, this->combo-1);
}

void Ingame::AddLife(){
    this->lifes++;
    this->RefreshTextSurface(&this->textLife, this->lifes);
}

void Ingame::RemoveLife(GAME_STATE* gs){
    this->lifes--;
    this->SetCombo(1);
    if(this->lifes < 0){
        *gs = GAMEOVER;
        this->RefreshTextSurface(&this->textLife, 0);
    }else{
        this->RefreshTextSurface(&this->textLife, this->lifes);
    }
}

void Ingame::RefreshTextSurface(SDL_Surface** surf, int value){
    if(*surf != NULL){
        SDL_FreeSurface(*surf);
    }
    *surf = this->GetIntTextSurface(value);
}

SDL_Surface* Ingame::GetIntTextSurface(int number){
    std::ostringstream s;
	s << number;
    return TTF_RenderUTF8_Solid(this->gameFont, s.str().c_str(), WHITE);
}

int Ingame::GetRandom(int limit){
    return rand()%limit+1;
}

void Ingame::Draw(SDL_Surface* viewport){
	this->player.Draw(viewport, RED);
	for(unsigned int i = 0; i < this->enemies.size(); i++){
		this->enemies[i]->Draw(viewport);
	}
    this->DrawUi(viewport);
}

void Ingame::DrawUi(SDL_Surface* viewport){
    SDL_Rect destR;
    destR.x = 5;
    destR.y = 0;
    destR.w = this->textLife->w;
    destR.h = this->textLife->h;
    SDL_BlitSurface(this->textLife, NULL, viewport, &destR);

    destR.x = SCREEN_WIDTH/2;
    destR.y = 0;
    destR.w = this->textScore->w;
    destR.h = this->textScore->h;
    SDL_BlitSurface(this->textScore, NULL, viewport, &destR);

    destR.x = SCREEN_WIDTH - 30;
    destR.y = 0;
    destR.w = this->textCombo->w;
    destR.h = this->textCombo->h;
    SDL_BlitSurface(this->textCombo, NULL, viewport, &destR);
}