/*
 * =====================================================================================
 *
 *       Filename:  helloworld.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/10/2017 10:46:39 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __HELLOWORLD_MOD_IOCTL_H__
#define __HELLOWORLD_MOD_IOCTL_H__

#define HELLOWORLD_MAGIC    12
#define HELLOWORLD_IOCTL_RESETLANG    _IO(HELLOWORLD_MAGIC,0)        //set langtype = english
#define HELLOWORLD_IOCTL_GETLANG        _IOR(HELLOWORLD_MAGIC,1,int)    //get langtype
#define HELLOWORLD_IOCTL_SETLANG        _IOW(HELLOWORLD_MAGIC,2,int)    //set langtype

typedef enum _lang_t
{
    english, chinese, pinyin
}lang_t;

#endif

