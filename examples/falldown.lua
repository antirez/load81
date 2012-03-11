-- falldown, lua edition \o/
-- © 2k12 r043v/dph


-- init function, launched at start
function setup()
 -- game option
 game = { map  = { data = {}, sizey = 6000, speed = 450, scroll = 0, bg = {255,0,255}, tile = { size = { x=50, y=50 }, color  = { {255,0,0},{0,255,0},{0,0,255},{255,255,0},{0,255,255} } } },
          hero = { isFall=1, color = {255,0,0}, size = { x=32, y=32 }, pos = { x=0, y=0 } },
          screen = { size = { x=0, y=0 }, fps = 30 },
        }

 -- init hero position
 game.hero.pos.x = math.floor( (WIDTH/2)+(game.hero.size.x/2) )
 game.hero.pos.y = -20--(hero.size.y)
 
 -- compute tile number by line and colon
 game.screen.size.x = math.floor( WIDTH / game.map.tile.size.x )
 game.screen.size.y = math.floor( HEIGHT / game.map.tile.size.y )
 
 -- max percent of line filled
 game.map.limit = math.floor(game.screen.size.x*(90/100))
 
 -- compute initial speed
 game.hero.oldSpeed = 0
 game.map.oldSpeed = 0
 game.map.speedRatio = 0
 game.hero.speedRatio = 0
 game.screen.frameFlip = 0
 
 refreshHeroSpeed()
 
 -- generate a random map
 generateMap()
 
 -- get ready for action!
end

function refreshHeroSpeed()
 game.hero.speed = math.floor(game.map.speed*1.1);
end

-- loop function
function draw()
 -- clear screen
 background(game.map.bg[1],game.map.bg[2],game.map.bg[3]);
 
 -- detect speed change and change current map/hero speed
 if game.map.oldSpeed ~= game.speed or game.hero.oldSpeed ~= hero.speed then
  game.map.oldSpeed = game.map.speed
  game.hero.oldhSpeed = game.hero.speed
  game.map.speedRatio = game.map.speed/game.screen.fps
  game.hero.speedRatio = (game.map.speed+game.hero.speed)/game.screen.fps
 end
 
 -- compute current move for hero and map in this frame
 game.screen.frameFlip = game.screen.frameFlip+1
 if game.screen.frameFlip >= game.screen.fps then game.screen.frameFlip = 0 end
 game.map.currentSpeed  = math.floor((game.screen.frameFlip+1)*game.map.speedRatio) - math.floor(game.screen.frameFlip*game.map.speedRatio)
 game.hero.currentSpeed = math.floor((game.screen.frameFlip+1)*game.hero.speedRatio) - math.floor(game.screen.frameFlip*game.hero.speedRatio)

 -- check keys
-- if keyboard.pressed['up'] then game.map.speed = game.map.speed+1; refreshHeroSpeed() end
-- if keyboard.pressed['down'] then game.map.speed = game.map.speed-1; refreshHeroSpeed() end

 if keyboard.pressed['left'] then
   local n
   for n=0,game.hero.currentSpeed/2,1 do
     local tiletopleft,tiletopright,bottomIndex,leftIndex,tiletopleft,tilebottomleft
     bottomIndex = math.floor((game.hero.pos.y+(game.hero.size.y-4))/game.map.tile.size.y)+2
     topIndex = math.floor((game.hero.pos.y+4)/game.map.tile.size.y)+2
     leftIndex = math.floor( (game.hero.pos.x-2)/game.map.tile.size.x)+1
     if leftIndex <= 0 then break end
     tiletopleft = game.map.data[topIndex][leftIndex]
     tilebottomleft = game.map.data[bottomIndex][leftIndex]
     if game.hero.pos.x > 0 and tilebottomleft+tiletopleft==0 then game.hero.pos.x = game.hero.pos.x-1 else break end
   end
 end

 if keyboard.pressed['right'] then
   local n
   for n=0,game.hero.currentSpeed/2,1 do
     local tiletopleft,tiletopright,bottomIndex,rightIndex,tiletopright,tilebottomright
     bottomIndex = math.floor((game.hero.pos.y+(game.hero.size.y-4))/game.map.tile.size.y)+2
     topIndex = math.floor( (game.hero.pos.y+4)/game.map.tile.size.y)+2
     rightIndex = math.floor( ((game.hero.pos.x+2)+game.hero.size.x)/game.map.tile.size.x )+1
     if rightIndex > game.screen.size.x then break end
     tiletopright = game.map.data[topIndex][rightIndex]
     tilebottomright = game.map.data[bottomIndex][rightIndex]
     if game.hero.pos.x < ( WIDTH-game.hero.size.x ) and tilebottomright+tiletopright==0  then game.hero.pos.x = game.hero.pos.x+1 else break end
   end
 end
 
 -- blit the map
 drawMap();
 
 -- blit hero²
 fill(game.hero.color[1],game.hero.color[2],game.hero.color[3],1);
 rect(game.hero.pos.x,HEIGHT-( (game.hero.pos.y+game.hero.size.y) - game.map.scroll),game.hero.size.x,game.hero.size.y);
 
 -- scroll the map
 game.map.scroll = game.map.scroll + game.map.currentSpeed
 
 -- go hero down
   local n
   for n=0,game.hero.currentSpeed,1 do
     -- check down tile
     local tileleft,tileright,bottomIndex,leftIndex,rightIndex
     bottomIndex = math.floor((game.hero.pos.y+game.hero.size.y)/game.map.tile.size.y)+2
     leftIndex = math.floor(game.hero.pos.x/game.map.tile.size.x)+1
     rightIndex = math.floor((game.hero.pos.x + (game.hero.size.x-1))/game.map.tile.size.x)+1
     tileleft = game.map.data[bottomIndex][leftIndex]
     tileright = game.map.data[bottomIndex][rightIndex]
     if tileleft + tileright > 0 then
       if game.hero.isFall == 1 then
     game.map.speed = game.map.speed+1; refreshHeroSpeed();
     game.hero.isFall=0
       end
       break
     end
     game.hero.pos.y = game.hero.pos.y + 1
     game.hero.isFall = 1
     --text(100,70,string.format("left %d right %d",tileleft,tileright))
   end
 
   if game.hero.pos.y - game.map.scroll < -500 then
     setup() -- game over !
   end
   
   
 -- some debug
-- fill(0,255,0,1)
-- text(100,100,string.format("scroll %d",game.map.scroll))
-- text(100,130,string.format("screen %d*%d => %d*%d",WIDTH,HEIGHT,game.screen.size.x,game.screen.size.y))
-- text(100,160,string.format("speed %d px/s current %d frame %d ratio %f",game.map.speed,game.map.currentSpeed,game.screen.frameFlip,game.map.speedRatio))
-- text(100,190,string.format("hero : %d speed %d",game.hero.pos.y,game.hero.currentSpeed))
end

function generateMap()
 local x,y
 local n=1
 local skip=0
 local count=0
 local current_color=0
 local nb_stop_cases=0
 
 -- init array
 for y=1,game.map.sizey,1 do
   game.map.data[y] = {}
   for x=1,game.screen.size.x,1 do
     game.map.data[y][x]=0
   end
 end
 
 -- generate start
 for y=1,game.screen.size.y,1 do
  game.map.data[y][1] = n
  game.map.data[y][game.screen.size.x] = n
  if n==1 then n=2 else n=1 end
 end
 
 -- generate other 
  for y=game.screen.size.y+1,game.map.sizey,1 do
    skip = skip+1
    if skip==4 then
      current_color = current_color+1
      if current_color>3 then current_color=2 end
      count = count+1
      if count%42 == 1 then current_color=4 end
      skip=0
      nb_stop_cases=0
      x=1
      while x<=game.screen.size.x do
    if nb_stop_cases<game.map.limit then
      if math.random(0,2)>0 then
        game.map.data[y][x] = current_color
        nb_stop_cases = nb_stop_cases+1
      else
        game.map.data[y][x] = 0 ;
      end
    else
      game.map.data[y][math.random(1,game.screen.size.x)]=0
      game.map.data[y][x] = 0 ;
    end
    x = x+1
      end
    end
  end
end

function drawMap()
 -- compute y start tile position, pixels -> tiles
 local starty = math.floor(game.map.scroll/game.map.tile.size.y)+1

 -- scroll is not bloc by bloc, compute first/last line height
 local decy = game.map.scroll % game.map.tile.size.y
 
 -- get number of line need to be blited
 local endy = starty+game.screen.size.y
 -- if first line is cliped, need to blit another one line at bottom, cliped too
 if(decy > 0) then endy = endy+1 end
 
 local nx,ny,tile,color,px,py
 py = HEIGHT+decy
 for ny=starty,endy,1 do
  px = 0
  for nx=1,game.screen.size.x,1 do
   tile = game.map.data[ny][nx]
   if(tile > 0) then
     color = game.map.tile.color[tile]
     fill(color[1],color[2],color[3],1)
     rect(px,py,game.map.tile.size.x,game.map.tile.size.y)  
   end
   px = px+game.map.tile.size.x
  end
  py = py-game.map.tile.size.y
 end
end
