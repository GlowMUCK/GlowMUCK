/*  
 *  heartbeat.c
 *  $Revision: 1.1 $ $Date: 2005/03/08 15:20:43 $
 *  We can pretend that MUF provides us a heartbeat through the use of a MUF
 *  dispatcher program in conjunction with process backgrounding. The result
 *  is less than desired, however. Adding a true heartbeat which can execute
 *  MUF programs individually is a preferred dispatching method.
 *  This eliminates the issues with the MUF programs failing or other was
 *  corrupting the stack for the master program.
 */

/* $Log: heartbeat.c,v $
/* Revision 1.1  2005/03/08 15:20:43  feaelin
/* Initial checkin
/* */

int heartbeat (void)
{
  /* Move Along, nothing to see here */  
}
