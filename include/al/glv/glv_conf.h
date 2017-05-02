#ifndef INC_GLV_CONF_H
#define INC_GLV_CONF_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#define	GLV_VERSION "0.9.7"
#define	GLV_MAX_MOUSE_BUTTONS 8


#define GLV_SNPRINTF snprintf

#ifdef __MINGW32__
	#undef GLV_SNPRINTF
	#define GLV_SNPRINTF _snprintf
#endif

#endif /* include guard */

