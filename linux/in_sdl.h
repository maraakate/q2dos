static struct {
  int key;
  int down;
} keyq[64];
static int keyq_head = 0;
static int keyq_tail = 0;

/*****************************************************************************/

int mx, my;

static float old_windowed_mouse;

static cvar_t	*_windowed_mouse;

void RW_IN_PlatformInit(void) {
  _windowed_mouse = ri.Cvar_Get ("_windowed_mouse", "0", CVAR_ARCHIVE);
}

void RW_IN_Activate(qboolean active)
{
  mx = my = 0;
}

/*****************************************************************************/

static int XLateKey(unsigned int keysym)
{
  int key;

  key = 0;
  switch(keysym) {
  case SDLK_KP9:		key = K_KP_PGUP; break;
  case SDLK_PAGEUP:		key = K_PGUP; break;

  case SDLK_KP3:		key = K_KP_PGDN; break;
  case SDLK_PAGEDOWN:		key = K_PGDN; break;

  case SDLK_KP7:		key = K_KP_HOME; break;
  case SDLK_HOME:		key = K_HOME; break;

  case SDLK_KP1:		key = K_KP_END; break;
  case SDLK_END:		key = K_END; break;

  case SDLK_KP4:		key = K_KP_LEFTARROW; break;
  case SDLK_LEFT:		key = K_LEFTARROW; break;

  case SDLK_KP6:		key = K_KP_RIGHTARROW; break;
  case SDLK_RIGHT:		key = K_RIGHTARROW; break;

  case SDLK_KP2:		key = K_KP_DOWNARROW; break;
  case SDLK_DOWN:		key = K_DOWNARROW; break;

  case SDLK_KP8:		key = K_KP_UPARROW; break;
  case SDLK_UP:			key = K_UPARROW; break;

  case SDLK_ESCAPE:		key = K_ESCAPE; break;

  case SDLK_KP_ENTER:		key = K_KP_ENTER; break;
  case SDLK_RETURN:		key = K_ENTER; break;

  case SDLK_TAB:		key = K_TAB; break;

  case SDLK_F1:			key = K_F1; break;
  case SDLK_F2:			key = K_F2; break;
  case SDLK_F3:			key = K_F3; break;
  case SDLK_F4:			key = K_F4; break;
  case SDLK_F5:			key = K_F5; break;
  case SDLK_F6:			key = K_F6; break;
  case SDLK_F7:			key = K_F7; break;
  case SDLK_F8:			key = K_F8; break;
  case SDLK_F9:			key = K_F9; break;
  case SDLK_F10:		key = K_F10; break;
  case SDLK_F11:		key = K_F11; break;
  case SDLK_F12:		key = K_F12; break;

  case SDLK_BACKSPACE:		key = K_BACKSPACE; break;

  case SDLK_KP_PERIOD:		key = K_KP_DEL; break;
  case SDLK_DELETE:		key = K_DEL; break;

  case SDLK_PAUSE:		key = K_PAUSE; break;

  case SDLK_LSHIFT:
  case SDLK_RSHIFT:		key = K_SHIFT; break;

  case SDLK_LCTRL:
  case SDLK_RCTRL:		key = K_CTRL; break;

  case SDLK_LMETA:
  case SDLK_RMETA:
  case SDLK_LALT:
  case SDLK_RALT:		key = K_ALT; break;

  case SDLK_KP5:		key = K_KP_5; break;

  case SDLK_INSERT:		key = K_INS; break;
  case SDLK_KP0:		key = K_KP_INS; break;

  case SDLK_KP_MULTIPLY:	key = '*'; break;
  case SDLK_KP_PLUS:		key = K_KP_PLUS; break;
  case SDLK_KP_MINUS:		key = K_KP_MINUS; break;
  case SDLK_KP_DIVIDE:		key = K_KP_SLASH; break;

    /* suggestions on how to handle this better would be appreciated */
  case SDLK_WORLD_7:		key = '`'; break;

  default: /* assuming that the other sdl keys are mapped to ascii */
    if (keysym < 128)
      key = keysym;
    break;
  }

  return key;
}

static unsigned char KeyStates[SDLK_LAST];

void getMouse(int *x, int *y, int *state) {
  *x = mx;
  *y = my;
  *state = mouse_buttonstate;
}

void doneMouse(void) {
  mx = my = 0;
}

static void GetEvent(SDL_Event *event)
{
	unsigned int key;

	switch(event->type) {
	case SDL_MOUSEBUTTONDOWN:
	  if (event->button.button == 4) {
	    keyq[keyq_head].key = K_MWHEELUP;
	    keyq[keyq_head].down = true;
	    keyq_head = (keyq_head + 1) & 63;
	    keyq[keyq_head].key = K_MWHEELUP;
	    keyq[keyq_head].down = false;
	    keyq_head = (keyq_head + 1) & 63;
	  } else if (event->button.button == 5) {
	    keyq[keyq_head].key = K_MWHEELDOWN;
	    keyq[keyq_head].down = true;
	    keyq_head = (keyq_head + 1) & 63;
	    keyq[keyq_head].key = K_MWHEELDOWN;
	    keyq[keyq_head].down = false;
	    keyq_head = (keyq_head + 1) & 63;
	  }
	  break;
	case SDL_MOUSEBUTTONUP:
	  break;

	case SDL_KEYDOWN:
	  if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
	    if (KeyStates[SDLK_LALT] || KeyStates[SDLK_RALT]) {
	      if (SDL_WM_ToggleFullScreen(surface)) {
		cvar_t *fullscreen;
		ri.Cvar_SetValue("vid_fullscreen", (surface->flags & SDL_FULLSCREEN)? 1 : 0);
		fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
		fullscreen->modified = false; /* we just changed it with SDL. */
	      }
	      break; /* ignore this key */
	    }
	  }

	  if (event->key.keysym.sym == SDLK_g && (KeyStates[SDLK_LCTRL] || KeyStates[SDLK_RCTRL])) {
	    SDL_GrabMode gm = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	    ri.Cvar_SetValue("_windowed_mouse", (gm == SDL_GRAB_ON)? SDL_GRAB_OFF : SDL_GRAB_ON);
	    break; /* ignore this key */
	  }

	  if (event->key.keysym.sym == SDLK_ESCAPE && (KeyStates[SDLK_LSHIFT] || KeyStates[SDLK_RSHIFT])) {
	    ri.Cmd_ExecuteText(EXEC_APPEND, "toggleconsole");
	    break; /* ignore this key */
	  }

	  KeyStates[event->key.keysym.sym] = 1;
	  key = XLateKey(event->key.keysym.sym);
	  if (key) {
	    keyq[keyq_head].key = key;
	    keyq[keyq_head].down = true;
	    keyq_head = (keyq_head + 1) & 63;
	  }
	  break;

	case SDL_KEYUP:
	  if (KeyStates[event->key.keysym.sym]) {
	    KeyStates[event->key.keysym.sym] = 0;
	    key = XLateKey(event->key.keysym.sym);
	    if (key) {
	      keyq[keyq_head].key = key;
	      keyq[keyq_head].down = false;
	      keyq_head = (keyq_head + 1) & 63;
	    }
	  }
	  break;

	case SDL_QUIT:
	  ri.Cmd_ExecuteText(EXEC_NOW, "quit");
	  break;
	}
}

/*****************************************************************************/

Key_Event_fp_t Key_Event_fp;

void RW_KBD_Init(Key_Event_fp_t fp)
{
  Key_Event_fp = fp;
}

void RW_KBD_Update(void)
{
  SDL_Event event;
  static int KBD_Update_Flag;

  in_state_t *in_state = getState();

  if (KBD_Update_Flag == 1)
    return;

  KBD_Update_Flag = 1;

  /* get events from SDL */
  if (rw_active)
    {
      int bstate;

      while (SDL_PollEvent(&event))
	GetEvent(&event);

      if (!mx && !my)
	SDL_GetRelativeMouseState(&mx, &my);
      mouse_buttonstate = 0;
      bstate = SDL_GetMouseState(NULL, NULL);
      if (SDL_BUTTON(1) & bstate)
	mouse_buttonstate |= (1 << 0);
      if (SDL_BUTTON(3) & bstate) /* quake2 has the right button be mouse2 */
	mouse_buttonstate |= (1 << 1);
      if (SDL_BUTTON(2) & bstate) /* quake2 has the middle button be mouse3 */
	mouse_buttonstate |= (1 << 2);
      if (SDL_BUTTON(6) & bstate)
	mouse_buttonstate |= (1 << 3);
      if (SDL_BUTTON(7) & bstate)
	mouse_buttonstate |= (1 << 4);

      if (old_windowed_mouse != _windowed_mouse->value) {
	old_windowed_mouse = _windowed_mouse->value;
	
	if (!_windowed_mouse->value) {
	  /* ungrab the pointer */
	  SDL_WM_GrabInput(SDL_GRAB_OFF);
	} else {
	  /* grab the pointer */
	  SDL_WM_GrabInput(SDL_GRAB_ON);
	}
      }
      while (keyq_head != keyq_tail)
	{
	  in_state->Key_Event_fp(keyq[keyq_tail].key, keyq[keyq_tail].down);
	  keyq_tail = (keyq_tail + 1) & 63;
	}
    }

  KBD_Update_Flag = 0;
}

void RW_KBD_Close(void)
{
	keyq_head = 0;
	keyq_tail = 0;
	memset(keyq, 0, sizeof(keyq));
}
