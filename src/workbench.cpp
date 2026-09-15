#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

struct RectF { float x; float y; float w; float h; };
enum class PartKind : std::size_t { Resistor=0, Capacitor, Led, Chip, Power, Connector };
struct PlacedPart { PartKind kind; RectF rect; };
struct Wire { std::size_t from; std::size_t to; };
constexpr std::array<const char*,6> kPartNames{"Resistor","Capacitor","LED","Chip","Power","Connector"};

void set_color(SDL_Renderer* r,std::uint8_t a,std::uint8_t b,std::uint8_t c,std::uint8_t d=255){SDL_SetRenderDrawColor(r,a,b,c,d);}
void fill_rect(SDL_Renderer* r,const RectF& q){const SDL_FRect x{q.x,q.y,q.w,q.h};SDL_RenderFillRectF(r,&x);}
void draw_rect(SDL_Renderer* r,const RectF& q){const SDL_FRect x{q.x,q.y,q.w,q.h};SDL_RenderDrawRectF(r,&x);}
bool contains(const RectF& q,float x,float y){return x>=q.x&&x<=q.x+q.w&&y>=q.y&&y<=q.y+q.h;}

RectF part_rect(PartKind k,float x,float y){switch(k){case PartKind::Resistor:return{x-34,y-13,68,26};case PartKind::Capacitor:return{x-22,y-24,44,48};case PartKind::Led:return{x-20,y-20,40,40};case PartKind::Chip:return{x-46,y-30,92,60};case PartKind::Power:return{x-36,y-26,72,52};case PartKind::Connector:return{x-24,y-42,48,84};}return{x-25,y-20,50,40};}

void draw_part(SDL_Renderer* r,const PlacedPart& p,bool selected){switch(p.kind){case PartKind::Resistor:set_color(r,190,132,69);break;case PartKind::Capacitor:set_color(r,56,122,209);break;case PartKind::Led:set_color(r,57,255,20);break;case PartKind::Chip:set_color(r,35,39,44);break;case PartKind::Power:set_color(r,196,58,58);break;case PartKind::Connector:set_color(r,201,210,214);break;}fill_rect(r,p.rect);set_color(r,selected?255:225,selected?225:232,selected?70:235);draw_rect(r,p.rect);const float cy=p.rect.y+p.rect.h*.5F;set_color(r,224,164,50);fill_rect(r,{p.rect.x-7,cy-3,7,6});fill_rect(r,{p.rect.x+p.rect.w,cy-3,7,6});}

struct WorkbenchState{PartKind selected_kind{PartKind::Resistor};std::vector<PlacedPart> parts;std::vector<Wire>wires;bool has_wire_start{false};std::size_t wire_start{0};bool validation_ran{false};bool validation_passed{false};};
float palette_width_for(float w){return std::min(230.0F,w*.24F);}
RectF board_rect_for(float w,float h){float p=palette_width_for(w),m=32;return{p+m,88,std::max(280.0F,w-p-m-m),std::max(220.0F,h-88-m)};}
RectF validate_button_for(float w){return{w-156,12,132,30};}
bool is_load(PartKind k){return k==PartKind::Resistor||k==PartKind::Led||k==PartKind::Chip||k==PartKind::Connector;}
void run_validation(WorkbenchState& s){s.validation_ran=true;s.validation_passed=false;for(const Wire& w:s.wires){if(w.from>=s.parts.size()||w.to>=s.parts.size())continue;PartKind a=s.parts[w.from].kind,b=s.parts[w.to].kind;if((a==PartKind::Power&&is_load(b))||(b==PartKind::Power&&is_load(a))){s.validation_passed=true;return;}}}
void update_window_title(SDL_Window* w,const WorkbenchState&s){std::string t="FormFactor prototype | Pick: ";t+=kPartNames[static_cast<std::size_t>(s.selected_kind)];t+=" | Left place | Right connect | V test | C clear";if(s.has_wire_start)t+=" | CONNECT: choose second part";else if(s.validation_ran)t+=s.validation_passed?" | TEST: PASS - powered load":" | TEST: NEED POWER + LOAD + WIRE";SDL_SetWindowTitle(w,t.c_str());}

void draw_workbench(SDL_Renderer*r,int width,int height,const WorkbenchState&s){float w=static_cast<float>(width),h=static_cast<float>(height);set_color(r,10,13,15);SDL_RenderClear(r);set_color(r,18,24,27);fill_rect(r,{0,0,w,56});set_color(r,57,255,20);fill_rect(r,{0,54,w,2});RectF vb=validate_button_for(w);if(s.validation_ran&&s.validation_passed)set_color(r,57,255,20);else if(s.validation_ran)set_color(r,210,65,65);else set_color(r,38,86,48);fill_rect(r,vb);set_color(r,225,232,235);draw_rect(r,vb);float pw=palette_width_for(w);set_color(r,15,20,22);fill_rect(r,{0,56,pw,h-56});const std::array<float,6> ys{92,146,200,254,308,362};for(std::size_t i=0;i<ys.size();++i){RectF slot{24,ys[i],pw-48,38};set_color(r,i==static_cast<std::size_t>(s.selected_kind)?37:28,i==static_cast<std::size_t>(s.selected_kind)?73:36,i==static_cast<std::size_t>(s.selected_kind)?45:39);fill_rect(r,slot);set_color(r,57,255,20);draw_rect(r,slot);PartKind k=static_cast<PartKind>(i);draw_part(r,{k,part_rect(k,45,ys[i]+19)},false);}RectF board=board_rect_for(w,h);set_color(r,18,54,43);fill_rect(r,board);set_color(r,57,255,20);draw_rect(r,board);set_color(r,28,78,63);for(float x=board.x+24;x<board.x+board.w;x+=24)SDL_RenderDrawLineF(r,x,board.y,x,board.y+board.h);for(float y=board.y+24;y<board.y+board.h;y+=24)SDL_RenderDrawLineF(r,board.x,y,board.x+board.w,y);set_color(r,232,117,17);for(const Wire& z:s.wires){if(z.from>=s.parts.size()||z.to>=s.parts.size())continue;const RectF&a=s.parts[z.from].rect,&b=s.parts[z.to].rect;SDL_RenderDrawLineF(r,a.x+a.w*.5F,a.y+a.h*.5F,b.x+b.w*.5F,b.y+b.h*.5F);}for(std::size_t i=0;i<s.parts.size();++i)draw_part(r,s.parts[i],s.has_wire_start&&i==s.wire_start);set_color(r,12,17,19);fill_rect(r,{board.x,h-46,board.w,28});if(s.validation_ran&&s.validation_passed){set_color(r,57,255,20);fill_rect(r,{board.x,h-46,board.w,3});}else if(s.validation_ran){set_color(r,210,65,65);fill_rect(r,{board.x,h-46,board.w*.35F,3});}else{set_color(r,57,255,20);fill_rect(r,{board.x,h-46,board.w*.12F,3});}SDL_RenderPresent(r);}

bool pick_palette(WorkbenchState&s,float x,float y,float width){float pw=palette_width_for(width);const std::array<float,6>ys{92,146,200,254,308,362};for(std::size_t i=0;i<ys.size();++i){if(contains({24,ys[i],pw-48,38},x,y)){s.selected_kind=static_cast<PartKind>(i);s.validation_ran=false;return true;}}return false;}
bool find_part(const WorkbenchState&s,float x,float y,std::size_t&i){for(std::size_t n=s.parts.size();n>0;--n){std::size_t c=n-1;if(contains(s.parts[c].rect,x,y)){i=c;return true;}}return false;}
void place_part(WorkbenchState&s,float x,float y,const RectF&b){RectF q=part_rect(s.selected_kind,x,y);q.x=std::clamp(q.x,b.x+8,b.x+b.w-q.w-8);q.y=std::clamp(q.y,b.y+8,b.y+b.h-q.h-8);s.parts.push_back({s.selected_kind,q});s.validation_ran=false;}
void connect_part(WorkbenchState&s,std::size_t i){if(!s.has_wire_start){s.has_wire_start=true;s.wire_start=i;return;}if(s.wire_start!=i){bool duplicate=false;for(const Wire&w:s.wires)if((w.from==s.wire_start&&w.to==i)||(w.from==i&&w.to==s.wire_start)){duplicate=true;break;}if(!duplicate){s.wires.push_back({s.wire_start,i});s.validation_ran=false;}}s.has_wire_start=false;}
}

int main(int argc,char*argv[]){(void)argc;(void)argv;if(SDL_Init(SDL_INIT_VIDEO)!=0){SDL_Log("SDL_Init failed: %s",SDL_GetError());return 1;}SDL_Window*window=SDL_CreateWindow("FormFactor prototype",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280,800,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);if(!window){SDL_Log("SDL_CreateWindow failed: %s",SDL_GetError());SDL_Quit();return 1;}SDL_Renderer*r=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);if(!r)r=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);if(!r){SDL_DestroyWindow(window);SDL_Quit();return 1;}WorkbenchState s;update_window_title(window,s);bool running=true;while(running){SDL_Event e{};while(SDL_PollEvent(&e)!=0){if(e.type==SDL_QUIT)running=false;else if(e.type==SDL_KEYDOWN){if(e.key.keysym.sym==SDLK_ESCAPE)running=false;else if(e.key.keysym.sym==SDLK_c)s=WorkbenchState{};else if(e.key.keysym.sym==SDLK_v)run_validation(s);update_window_title(window,s);}else if(e.type==SDL_MOUSEBUTTONDOWN){int wi=0,he=0;SDL_GetRendererOutputSize(r,&wi,&he);float x=static_cast<float>(e.button.x),y=static_cast<float>(e.button.y),w=static_cast<float>(wi),h=static_cast<float>(he);RectF board=board_rect_for(w,h);if(e.button.button==SDL_BUTTON_LEFT){if(contains(validate_button_for(w),x,y))run_validation(s);else if(!pick_palette(s,x,y,w)&&contains(board,x,y))place_part(s,x,y,board);}else if(e.button.button==SDL_BUTTON_RIGHT){std::size_t i=0;if(find_part(s,x,y,i))connect_part(s,i);}update_window_title(window,s);}}int wi=0,he=0;SDL_GetRendererOutputSize(r,&wi,&he);draw_workbench(r,wi,he,s);}SDL_DestroyRenderer(r);SDL_DestroyWindow(window);SDL_Quit();return 0;}
