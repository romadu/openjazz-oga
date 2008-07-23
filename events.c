
/*
 *
 * events.c
 * Created on the 1st of January 2006
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2006 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Deals with events in ordinary levels.
 *
 */


#include "level.h"


void createPlayerBullet (player *source, int ticks) {

  bullet *bul;

  if ((source->ammoType == -1) ||
      (source->ammo[source->ammoType])) {

    if (source->ammoType != -1)
      source->ammo[source->ammoType]--;

    if (unusedBullet) {

      bul = unusedBullet;
      unusedBullet = unusedBullet->next;

    } else bul = malloc(sizeof(bullet));

    bul->next = firstBullet;
    bul->origin = 0;

    switch (source->ammoType) {

      case -1:

        bul->type = B_BLASTER;

        break;

      case 0:

        if (source->facing) bul->type = B_RTOASTER;
        else bul->type = B_LTOASTER;

        break;

      case 1:

        if (source->facing) bul->type = B_RMISSILE;
        else bul->type = B_LMISSILE;

        break;

      case 2:

        bul->type = B_BOUNCER;

        break;

      case 3:

        bul->type = B_TNT;

        break;

    }

    bul->x = source->x + F6 + (F20 * source->facing);
    bul->y = source->y - F8;

    if ((bul->type == B_LMISSILE) || (bul->type == B_RMISSILE)) {

      if (source->facing) {

        bul->dx = 500 * F1;
        bul->dy = -100 * F1;

      } else {

        bul->dx = -500 * F1;
        bul->dy = -100 * F1;

      }

      bul->time = ticks + 1000;
      firstBullet = bul;

      if (unusedBullet) {

        bul = unusedBullet;
        unusedBullet = unusedBullet->next;

      } else bul = malloc(sizeof(bullet));

      bul->next = firstBullet;
      bul->origin = 0;

      if (source->facing) bul->type = B_RMISSILE;
      else bul->type = B_LMISSILE;

      bul->x = source->x + F6 + (F20 * source->facing);
      bul->y = source->y - F8;

      if (source->facing) {

        bul->dx = 500 * F1;
        bul->dy = 100 * F1;

      } else {

        bul->dx = -500 * F1;
        bul->dy = 100 * F1;

      }

    } else {

      if (bul->type == B_TNT) bul->dx = 0;
      else {

        if (!source->facing) {

          bul->dx = -500 * F1;

        } else {

          bul->dx = 500 * F1;

        }

      }

      bul->dy = 0;

    }

    bul->time = ticks + 3000;
    firstBullet = bul;

  }

  return;

}

void createEventBullet (event *evt, int ticks) {

  return;

}

void removeBullet (bullet * previous) {

  bullet *bul;

  if (previous) {

    bul = previous->next;
    previous->next = bul->next;

  } else {

    bul = firstBullet;
    firstBullet = bul->next;

  }

  // Submit for recycling
  bul->next = unusedBullet;
  unusedBullet = bul;

}


void freeBullets (void) {

  bullet *bul;

  while (firstBullet) {

    bul = firstBullet->next;
    free(firstBullet);
    firstBullet = bul;

  }

  while (unusedBullet) {

    bul = unusedBullet->next;
    free(unusedBullet);
    unusedBullet = bul;

  }

  return;

}


void createEvent (int x, int y) {

  event *evt;

  if (unusedEvent) {

    evt = unusedEvent;
    unusedEvent = unusedEvent->next;

  } else evt = malloc(sizeof(event));

  evt->gridX = x;
  evt->gridY = y;
  evt->x = x << 15;
  evt->y = (y + 1) << 15;
  evt->anim = 0;
  evt->flashTime = 0;

  if (firstEvent) firstEvent->prev = evt;
  evt->next = firstEvent;
  evt->prev = NULL;
  firstEvent = evt;

  return;

}


void removeEvent (event * evt) {

  if (evt->prev) {

    evt->prev->next = evt->next;

    if (evt->next) evt->next->prev = evt->prev;

  } else {

    firstEvent = evt->next;

    if (firstEvent) firstEvent->prev = NULL;

  }

  // Submit for recycling
  evt->next = unusedEvent;
  unusedEvent = evt;

  return;

}


void freeEvents (void) {

  event *evt;

  while (firstEvent) {

    evt = firstEvent->next;
    free(firstEvent);
    firstEvent = evt;

  }

  while (unusedEvent) {

    evt = unusedEvent->next;
    free(unusedEvent);
    unusedEvent = evt;

  }

  return;

}


void playEventFrame (event * evt, int ticks) {

  struct {
    fixed x, y, w, h;
  } pos;
  bullet *bul, *prevBul;
  gridElement *ge;
  signed char *set;
  int frame;
  fixed startX, targetH;
  int midpoint;

  // Get the grid element at the given coordinates
  ge = grid[evt->gridY] + evt->gridX;

  // Check whether or not the event should be processed
  if (!ge->event || (ge->event >= 122)) return;

  set = eventSet[ge->event];


  // Find frame and dimensions
  if (evt->anim && (set[evt->anim] >= 0)) {

    if (evt->anim == E_FINISHANIM)
      frame = ((ticks + 200 - ge->time) / 40) % animSet[set[evt->anim]].frames;
    else if (set[E_ANIMSP])
      frame = (ticks / (set[E_ANIMSP] * 40)) % animSet[set[evt->anim]].frames;
    else frame = (ticks / 40) % animSet[set[evt->anim]].frames;

    pos.x = evt->x + (animSet[set[evt->anim]].sprites[frame].x << 10);
    pos.y = evt->y +
            ((animSet[set[evt->anim]].sprites[frame].y +
              animSet[set[evt->anim]].y[frame] -
              animSet[set[evt->anim]].sprites[0].pixels->h) << 10);
    pos.w = animSet[set[evt->anim]].sprites[0].pixels->w << 10;
    pos.h = evt->y - pos.y;
    targetH = animSet[set[evt->anim]].sprites[0].pixels->h << 10;

    // Blank sprites for e.g. invisible springs
    if ((pos.w == F1) && (pos.h == F1)) pos.w = F32;

  } else {

    pos.x = evt->x;
    pos.y = evt->y - F32;
    pos.w = F32;
    pos.h = F32;
    targetH = F32;

  }

  // Deal with bullet collisions
  if ((evt->anim != E_FINISHANIM) && (ge->hits < set[E_HITSTOKILL])) {

    bul = firstBullet;
    prevBul = NULL;

    while (bul) {

      if ((bul->x > pos.x) && (bul->x < pos.x + pos.w) &&
          (bul->y > pos.y) && (bul->y < pos.y + targetH)   ) {

        ge->hits++;
        evt->flashTime = ticks + FLASHTIME;
        bul = bul->next;
        removeBullet(prevBul);

      } else {

        prevBul = bul;
        bul = bul->next;

      }

    }

  }


  startX = evt->x;

  // Handle behaviour
  if (evt->anim != E_FINISHANIM) {

    switch (set[E_BEHAVIOUR]) {

      case 0: // Face the player

        if (localPlayer->x + F16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        break;

      case 1: // Sink down

        if (localPlayer->x + F16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        evt->y += 320 * mspf / set[E_MOVEMENTSP];

        break;

      case 2: // Walk from side to side

        // Walk from side to side
        if (evt->anim == E_LEFTANIM) {

          if (!checkMaskDown(pos.x, pos.y + pos.h + F4) ||
              checkMaskDown(pos.x - F4, pos.y + (pos.h >> 1)))
            evt->anim = E_RIGHTANIM;

          evt->x -= 320 * mspf / set[E_MOVEMENTSP];

        } else if (evt->anim == E_RIGHTANIM) {

          if (!checkMaskDown(pos.x + pos.w, pos.y + pos.h + F4) ||
              checkMaskDown(pos.x + pos.w + F4, pos.y + (pos.h >> 1)))
            evt->anim = E_LEFTANIM;

          evt->x += 320 * mspf / set[E_MOVEMENTSP];

        } else evt->anim = E_LEFTANIM;

        break;

      case 3: // Seek jazz

        if (localPlayer->x + F26 < pos.x) {

          evt->anim = E_LEFTANIM;
          evt->x -= 320 * mspf / set[E_MOVEMENTSP];

        } else if (localPlayer->x + F6 > pos.x + pos.w) {

          evt->anim = E_RIGHTANIM;
          evt->x += 320 * mspf / set[E_MOVEMENTSP];

        } else if (!evt->anim) evt->anim = E_RIGHTANIM;

        break;

      case 4: // Walk from side to side and down hills

        if (!checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h)) {

          // Fall downwards
          evt->y += 320 * mspf / set[E_MOVEMENTSP];

        } else {

          // Walk from side to side
          if (evt->anim == E_LEFTANIM) {

            if (checkMaskDown(pos.x - F4, pos.y + (pos.h >> 1)))
              evt->anim = E_RIGHTANIM;

            evt->x -= 320 * mspf / set[E_MOVEMENTSP];

          } else if (evt->anim == E_RIGHTANIM) {

            if (checkMaskDown(pos.x + pos.w + F4, pos.y + (pos.h >> 1)))
              evt->anim = E_LEFTANIM;

            evt->x += 320 * mspf / set[E_MOVEMENTSP];

          } else evt->anim = E_LEFTANIM;

        }

        break;

      case 6: // Use the path from the level file

        // Check movement direction
        if ((pathNode < 3) || (pathX[pathNode] <= pathX[pathNode - 3]))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        // Choose new position
        evt->x = (evt->gridX << 15) + F16 + (pathX[pathNode] << 12);
        evt->y = (evt->gridY << 15) + (pathY[pathNode] << 11);

        break;

      case 7: // Move back and forth horizontally with tail

        if (evt->anim == E_LEFTANIM) {

          evt->x -= 80 * mspf / set[E_MOVEMENTSP];

          if (evt->x < evt->gridX << 15) evt->anim = E_RIGHTANIM;

        } else if (evt->anim == E_RIGHTANIM) {

          evt->x += 80 * mspf / set[E_MOVEMENTSP];

          if (evt->x > (evt->gridX << 15) + 100 * F1) evt->anim = E_LEFTANIM;

        } else evt->anim = E_LEFTANIM;

        break;

      case 8: // Bird-esque following

        if (localPlayer->dx > 0) {

          evt->anim = E_RIGHTANIM;

          if (evt->x < localPlayer->x) evt->x += 320 * mspf / set[E_MOVEMENTSP];

        } else if (localPlayer->dx < 0) {

          evt->anim = E_LEFTANIM;

          if (evt->x > localPlayer->x) evt->x -= 320 * mspf / set[E_MOVEMENTSP];

        } else evt->anim = E_LEFTANIM + localPlayer->facing;

        if (evt->y > localPlayer->y - F64)
          evt->y -= 320 * mspf / set[E_MOVEMENTSP];
        else if (evt->y < localPlayer->y - F64)
          evt->y += 320 * mspf / set[E_MOVEMENTSP];

        break;

      case 11: // Sink to ground

        if (localPlayer->x + F16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        if (!checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h))
          evt->y += 320 * mspf / set[E_MOVEMENTSP];

        break;

      case 12: // Move back and forth horizontally

        if (evt->anim == E_LEFTANIM) {

          if (checkMaskDown(pos.x - F4, pos.y + (pos.h >> 1)))
            evt->anim = E_RIGHTANIM;

          evt->x -= 80 * mspf / set[E_MOVEMENTSP];

        } else if (evt->anim == E_RIGHTANIM) {

          if (checkMaskDown(pos.x + pos.w + F4, pos.y + (pos.h >> 1)))
            evt->anim = E_LEFTANIM;

          evt->x += 80 * mspf / set[E_MOVEMENTSP];

        } else evt->anim = E_LEFTANIM;

        break;

      case 13: // Move up and down

        if (evt->anim == E_LEFTANIM) {

          if (checkMaskDown(pos.x + (pos.w >> 1), pos.y - F4))
            evt->anim = E_RIGHTANIM;

          evt->y -= 80 * mspf / set[E_MOVEMENTSP];

        } else if (evt->anim == E_RIGHTANIM) {

          if (checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h + F4))
            evt->anim = E_LEFTANIM;

          evt->y += 80 * mspf / set[E_MOVEMENTSP];

        } else evt->anim = E_LEFTANIM;

        break;

      case 16: // Move across level to the left or right

        evt->x += set[E_MAGNITUDE] * 80 * mspf / set[E_MOVEMENTSP];
        evt->anim = E_LEFTANIM;

        break;

      case 21: // Destructible block

        if (ge->hits >= set[E_HITSTOKILL]) ge->tile = set[E_MULTIPURPOSE];
        break;

      case 26: // Flip animation

        evt->anim = E_RIGHTANIM;

        break;

      case 31: // Moving platform

        if (evt->x < (evt->gridX << 15) - (set[E_BRIDGELENGTH] << 14))
          evt->anim = E_RIGHTANIM;
        else if ((evt->x > ((evt->gridX + set[E_BRIDGELENGTH]) << 15)) ||
                 (evt->anim == 0)                                     )
          evt->anim = E_LEFTANIM;

        if (evt->anim == E_LEFTANIM) evt->x -= 320 * mspf / set[E_MOVEMENTSP];
        else evt->x += 320 * mspf / set[E_MOVEMENTSP];

        break;

      case 33: // Sparks-esque following

        if (localPlayer->facing && (pos.x + pos.w < localPlayer->x)) {

          evt->x += 320 * mspf / set[E_MOVEMENTSP];
          evt->anim = E_RIGHTANIM;

          if (pos.y + pos.h < localPlayer->y - F20)
            evt->y += 80 * mspf / set[E_MOVEMENTSP];

          else if (pos.y > localPlayer->y)
            evt->y -= 80 * mspf / set[E_MOVEMENTSP];

        } else if (!localPlayer->facing && (pos.x > localPlayer->x + F32)) {

          evt->x -= 320 * mspf / set[E_MOVEMENTSP];
          evt->anim = E_LEFTANIM;

          if (pos.y + pos.h < localPlayer->y - F20)
            evt->y += 80 * mspf / set[E_MOVEMENTSP];

          else if (pos.y > localPlayer->y)
            evt->y -= 80 * mspf / set[E_MOVEMENTSP];

        } else if (!evt->anim) evt->anim = E_LEFTANIM;

        break;

      /* As yet unhandled event behaviours follow */

      case 36: // Walk from side to side and down hills, staying on-screen

        if (!checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h)) {

          // Fall downwards
          evt->y += 320 * mspf / set[E_MOVEMENTSP];

        } else {

          // Walk from side to side, staying on-screen
          if (evt->anim == E_LEFTANIM) {

            if (checkMaskDown(pos.x - F4, pos.y + (pos.h >> 1)) ||
                (pos.x - F4 < localPlayer->viewX))
              evt->anim = E_RIGHTANIM;

            evt->x -= 320 * mspf / set[E_MOVEMENTSP];

          } else if (evt->anim == E_RIGHTANIM) {

            if (checkMaskDown(pos.x + pos.w + F4, pos.y + (pos.h >> 1)) ||
                (pos.x + pos.w + F4 >
                 localPlayer->viewX + (localPlayer->viewW << 10)))
              evt->anim = E_LEFTANIM;

            evt->x += 320 * mspf / set[E_MOVEMENTSP];

          } else evt->anim = E_LEFTANIM;

        }

        break;

      case 14: // Move back and forth rapidly
      case 15: // Rise or lower to meet jazz
      case 22: // Fall down in random spot and repeat
      case 24: // Crawl along ground and go downstairs
      case 27: // Face jazz
      case 28: // Bridge
      case 29: // Nonmoving object with jazz
      case 30: // Nonmoving object with jazz
      case 32: // Nonmoving object
      case 34: // Launching platform
      case 39: // Collapsing floor
      case 41: // Switch left & right anim periodically
      case 44: // Leap to greet Jazz very quickly
      case 46: // "Final" boss
      case 47:
      case 48:
      case 49:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 56:
      case 59:
      case 60:
      case 61:
      case 62:
      case 65:

        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        break;

    }

  }


  // If the event has been destroyed, play its finishing animation and set its
  // reaction time
  if (set[E_HITSTOKILL] && (evt->anim != E_FINISHANIM) &&
     (ge->hits >= set[E_HITSTOKILL])                     ) {

    localPlayer->score += set[E_ADDEDSCORE] * 10;
    ge->time = ticks + 200;
    evt->anim = E_FINISHANIM;

  }


  // If the reaction time has expired
  if (ge->time && (ticks > ge->time)) {

    // Handle modifiers which take effect after reaction time
    switch (set[E_MODIFIER]) {

      case 41: // Bonus level

        if (localPlayer->reaction != PR_KILLED) {

          free(nextLevel);
          nextLevel = malloc(11);
          sprintf(nextLevel, "level%1i.%03i", set[E_MULTIPURPOSE], set[E_YAXIS]);

        }

        // The lack of a break statement is intentional

      case 8: // Boss
      case 27: // End of level

        if (localPlayer->reaction != PR_KILLED) {

          checkX = evt->gridX;
          checkY = evt->gridY;

          localPlayer->reaction = PR_WON;
          localPlayer->reactionTime = ticks + 2000;

        }

        break;

      case 10: // Checkpoint

        checkX = evt->gridX;
        checkY = evt->gridY;

        break;

      case 13: // Warp

        localPlayer->x = set[E_MULTIPURPOSE] << 15;
        localPlayer->y = (set[E_YAXIS] + 1) << 15;
        ge->time = 0;

        break;

      case 15:

        if (!localPlayer->ammo[0]) localPlayer->ammoType = 0;
        localPlayer->ammo[0] += 15;
        break;

      case 16:

        if (!localPlayer->ammo[1]) localPlayer->ammoType = 1;
        localPlayer->ammo[1] += 15;
        break;

      case 17:

        if (!localPlayer->ammo[2]) localPlayer->ammoType = 2;
        localPlayer->ammo[2] += 15;
        break;

      case 34: // Bird

        ge->event = 121;
        removeEvent(evt);
        
        return;

    }

    if (evt->anim == E_FINISHANIM) {

      ge->event = 0;
      removeEvent(evt);

      return;

    } else {

      ge->time = 0;

    }

  }


  // Handle contact events
  if ((evt->anim != E_FINISHANIM) &&
      (localPlayer->x + F6 <= pos.x + pos.w) &&
      (localPlayer->x + F26 >= pos.x) &&
      (localPlayer->y - F20 <= pos.y + pos.h) &&
      (localPlayer->y >= pos.y)   ) {

    // Handle behaviour which takes effect on contact
    switch (set[E_BEHAVIOUR]) {

      case 26: // Flip animation

        evt->anim = E_LEFTANIM;

        break;

      case 25: // Float up / Belt

        if (set[E_YAXIS]) {
 
          if ((localPlayer->dy > 0) &&
              checkMaskDown(localPlayer->x + F16, localPlayer->y + F4))
            localPlayer->dy = set[E_MULTIPURPOSE] * -F40;

          if (localPlayer->dy > set[E_MULTIPURPOSE] * -F40)
            localPlayer->dy -= set[E_MULTIPURPOSE] * 320 * mspf;

          localPlayer->event = ge->event;
          localPlayer->jumpY = localPlayer->y - (8 * F16);

        }

        if (set[E_MAGNITUDE] < 0)
          localPlayer->dx += set[E_MAGNITUDE] * 4 * mspf;
        else localPlayer->dx += set[E_MAGNITUDE] * 40 * mspf;

        break;

      case 38: // Sucker tubes

        if (set[E_YAXIS]) localPlayer->dy = set[E_MULTIPURPOSE] * -F20;

        localPlayer->dx = set[E_MAGNITUDE] * F40;

        break;

    }

    // Handle modifiers which take effect on contact
    switch (set[E_MODIFIER]) {

      case 0: // Hurt
      case 8: // Boss

        if ((localPlayer->reaction == PR_NONE) &&
            ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44))) {

          localPlayer->energy--;

          if (localPlayer->energy) {

            localPlayer->reaction = PR_HURT;
            localPlayer->reactionTime = ticks + 1000;

            if (localPlayer->dx < 0) {

              localPlayer->dx = PS_RUN;
              localPlayer->dy = PS_JUMP;

            } else {

              localPlayer->dx = -PS_RUN;
              localPlayer->dy = PS_JUMP;

            }

          } else {

            localPlayer->reaction = PR_KILLED;
            localPlayer->reactionTime = ticks + 2000;

          }

        }

        break;

      case 1: // Invincibility

        if ((localPlayer->reaction == PR_NONE) ||
            (localPlayer->reaction == PR_INVINCIBLE)) {

          localPlayer->reaction = PR_INVINCIBLE;
          localPlayer->reactionTime = ticks + 10000;

          localPlayer->score += set[E_ADDEDSCORE] * 10;
          ge->time = ticks + 200;
          evt->anim = E_FINISHANIM;

        }

        break;

      case 2:
      case 3: // Health

        if (localPlayer->energy < 4) localPlayer->energy++;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 4: // Extra life

        if (localPlayer->lives < 99) localPlayer->lives++;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 5: // High-jump feet

        localPlayer->jumpHeight += F16;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 6: // Platform

        if ((localPlayer->y <= pos.y + F4) &&
            !checkMaskDown(localPlayer->x + F16, pos.y - F20)) {

          localPlayer->event = ge->event;
          localPlayer->y = pos.y;
          localPlayer->x += evt->x - startX;

        } else if (eventSet[localPlayer->event][E_MODIFIER] == 6)
          localPlayer->event = 0;

        break;

      case 9: // Sand timer

        endTicks += 2 * 60 * 1000; // 2 minutes. Is this right?
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 11: // Item

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 12: // Rapid fire

        localPlayer->fireSpeed++;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 13: // Warp

        ge->time = ticks + 1000;

        break;

      case 18:

        if (!localPlayer->ammo[0]) localPlayer->ammoType = 0;
        localPlayer->ammo[0] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 19:

        if (!localPlayer->ammo[1]) localPlayer->ammoType = 1;
        localPlayer->ammo[1] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 20:

        if (!localPlayer->ammo[2]) localPlayer->ammoType = 2;
        localPlayer->ammo[2] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 30:

        localPlayer->ammo[3]++;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 29: // Upwards spring

        localPlayer->event = ge->event;
        localPlayer->jumpY = evt->y + (set[E_MAGNITUDE] * F20);

        break;

      case 35: // Airboard, etc.

        localPlayer->floating = 1;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 38: // Airboard, etc. off

        localPlayer->floating = 0;

        break;

    }

  }

  // Handle bridges
  if (set[E_BEHAVIOUR] == 28) {

    if ((localPlayer->x + F6 <=
         pos.x + (set[E_MULTIPURPOSE] * F8) - F10) &&
        (localPlayer->x + F26 >= pos.x - F10) &&
        (localPlayer->y <= pos.y + F8 + F20 + (set[E_YAXIS] << 10) - F32) &&
        (localPlayer->y >= pos.y + (set[E_YAXIS] << 10) - F32)   ) {

      if (!checkMaskDown(localPlayer->x + F16,
                         pos.y + (set[E_YAXIS] << 10) - (F32 + F20))) {

        localPlayer->event = ge->event;
        localPlayer->y = pos.y + (set[E_YAXIS] << 10) - F32;

        midpoint = (localPlayer->x + F16 - pos.x) >> 13;

        if (midpoint < set[E_MULTIPURPOSE] / 2) {

          evt->y = ((evt->gridY + 1) << 15) + (midpoint << 10);
          evt->anim = E_LEFTANIM;

        } else {

          evt->y = ((evt->gridY + 1) << 15) +
                      ((set[E_MULTIPURPOSE] - midpoint) << 10);
          evt->anim = E_RIGHTANIM;

        }

      } else if (eventSet[localPlayer->event][E_BEHAVIOUR] == 28)
        localPlayer->event = 0;

    } else {

      midpoint = 0;
      if (evt->y > (evt->gridY + 1) << 15) evt->y -= 16 * mspf;

    }

  }

  return;

}


void drawEvent (event * evt, int ticks) {

  SDL_Rect dst;
  SDL_Surface *sprite;
  gridElement *ge;
  signed char *set;
  int frame;
  int count, dstx, dsty, midpoint;

  // Get the grid element at the given coordinates
  ge = grid[evt->gridY] + evt->gridX;

  // Check whether or not the event should be processed
  if (!ge->event || (ge->event >= 122)) return;

  set = eventSet[ge->event];


  // Note: Behaviour 7, like 28, will need additional stuff
  if (evt->anim && (set[evt->anim] >= 0)) {

    if (evt->anim == E_FINISHANIM)
      frame = ((ticks + 200 - ge->time) / 40) % animSet[set[evt->anim]].frames;
    else if (set[E_ANIMSP])
      frame = (ticks / (set[E_ANIMSP] * 40)) % animSet[set[evt->anim]].frames;
    else frame = (ticks / 20) % animSet[set[evt->anim]].frames;

    dst.x = dstx = (evt->x >> 10) - (localPlayer->viewX >> 10) +
                   animSet[set[evt->anim]].sprites[frame].x;
    dst.y = dsty = (evt->y >> 10) - (localPlayer->viewY >> 10) +
                   animSet[set[evt->anim]].sprites[frame].y +
                   animSet[set[evt->anim]].y[frame] -
                   animSet[set[evt->anim]].sprites[0].pixels->h;

    sprite = animSet[set[evt->anim]].sprites[frame].pixels;

    if (ticks < evt->flashTime) scalePalette(sprite, 0, 0);

    // Draw the event

    if (set[E_BEHAVIOUR] == 28) {

      dstx -= 10;
      dsty -= 32 - set[E_YAXIS];

      if (evt->y > (evt->gridY + 1) << 15) {

        if (evt->anim == E_LEFTANIM)
          midpoint = (evt->y - ((evt->gridY + 1) << 15)) >> 10;
        else midpoint = set[E_MULTIPURPOSE] -
                        ((evt->y - ((evt->gridY + 1) << 15)) >> 10);

      } else {

        midpoint = 0;

      }

      if (midpoint < set[E_MULTIPURPOSE] / 2) {

        for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

          dst.x = dstx += 8;
          if (midpoint == 0) dst.y = dsty;
          else if (count < midpoint)
            dst.y = ((dsty - midpoint) * (midpoint - count) / midpoint) +
                    (dsty * count / midpoint);
          else
            dst.y = ((dsty - midpoint) * (count - midpoint) /
                     (set[E_MULTIPURPOSE] - midpoint)) +
                    (dsty * (set[E_MULTIPURPOSE] - count) /
                     (set[E_MULTIPURPOSE] - midpoint));

          SDL_BlitSurface(sprite, NULL, screen, &dst);

        }

      } else {

        for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

          dst.x = dstx += 8;
          if (midpoint == 0) dst.y = dsty;
          else if (count < midpoint)
            dst.y = ((dsty + midpoint - set[E_MULTIPURPOSE]) * (midpoint - count) / midpoint) +
                    (dsty * count / midpoint);
          else
            dst.y = ((dsty + midpoint - set[E_MULTIPURPOSE]) * (count - midpoint) / (set[E_MULTIPURPOSE] - midpoint)) +
                    (dsty * (set[E_MULTIPURPOSE] - count) / (set[E_MULTIPURPOSE] - midpoint));

          SDL_BlitSurface(sprite, NULL, screen, &dst);

        }

      }

    } else {

      SDL_BlitSurface(sprite, NULL, screen, &dst);

    }

    if (ticks < evt->flashTime) restorePalette(sprite);

    // If the event has been shot, draw an explosion
    if (set[E_HITSTOKILL] && (evt->anim == E_FINISHANIM)) {

      dst.x = dstx +
              animSet[121].sprites[frame].x;
      dst.y = dsty +
              animSet[121].sprites[frame].y +
              animSet[121].y[frame];

      SDL_BlitSurface(animSet[121].sprites[frame].pixels, NULL, screen,
                    &dst);

    }

  }

  return;

}

