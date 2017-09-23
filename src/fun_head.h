/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
LSD is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/

/***************************************************
****************************************************
FUN_HEAD.CPP

This file contains all the declarations and macros available in a model's equation file.

****************************************************
****************************************************/

#define FUN
#include "decl.h"

double def_res = 0;										// default equation result

extern bool fast;										// flag to hide LOG messages & runtime
extern bool invalidHooks;								// flag to invalid hooks pointers (set by simulation)
extern bool use_nan;									// flag to allow using Not a Number value
extern char *path;										// folder where the configuration is
extern char *simul_name;								// configuration name being run (for saving networks)
extern int cur_sim;
extern int debug_flag;
extern int max_step;
extern int quit;
extern int ran_gen;										// pseudo-random number generator to use (1-5))
extern int seed;
extern int sim_num;
extern int t;
extern long idum;
extern lsdstack *stacklog;
extern object *root;

bool is_finite( double x );
bool is_inf( double x );
bool is_nan( double x );
double alapl( double mu, double alpha1, double alpha2 );// draw from an asymmetric laplace distribution
double alaplcdf( double mu, double alpha1, double alpha2, double x );	// asymmetric laplace cumulative distribution function
double beta( double alpha, double beta );				// draw from a beta distribution
double betacdf( double alpha, double beta, double x );	// beta cumulative distribution function
double betacf( double a, double b, double x );			// beta distribution function
double exp(double c);
double fact( double x );								// Factorial function
double gamdev( int ia, long *idum_loc = NULL );
double gamma(double m) {return gamdev((int)m, &idum);};
double init_lattice(double pixW, double pixH, double nrow, double ncol, char const lrow[], char const lcol[], char const lvar[], object *p, int init_color);
double lnorm( double mu, double sigma );				// draw from a lognormal distribution
double lnormcdf( double mu, double sigma, double x );	// lognormal cumulative distribution function
double log(double v);
double max(double a, double b);
double min(double a, double b);
double norm(double mean, double dev);
double normcdf( double mu, double sigma, double x );	// normal cumulative distribution function
double poidev( double xm, long *idum_loc = NULL );
double poisson(double m) {return poidev(m, &idum); };
double poissoncdf( double lambda, double k );			// poisson cumulative distribution function
double rnd_integer(double m, double x);
double round(double r);
double save_lattice( const char *fname );
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
double update_lattice(double line, double col, double val);
int deb(object *r, object *c, char const *lab, double *res);
object *get_cycle_obj( object *c, char const *label, char const *command );
object *go_brother(object *c);
void cmd( const char *cm, ... );
void error_hard( const char *logText, const char *boxTitle = "", const char *boxText = "" );
void init_random( int seed );							// reset the random number generator seed
void msleep( unsigned msec );							// sleep process for milliseconds
void plog( char const *msg, char const *tag = "", ... );
void results_alt_path( const char * );  				// change where results are saved.

// redefine as macro to avoid conflicts with C++ version in <cmath.h>
double _abs(double a)
{
	if( a > 0 )
		return a;
	else
		return( -1 * a );
};
#define abs( a ) _abs( a )

#ifndef NO_WINDOW
#include <tk.h>

extern Tcl_Interp *inter;
extern double i_values[];

#define DEBUG_CODE \
	if ( debug_flag ) \
	{ \
		for ( i = 0; i < 1000; i++ ) \
			i_values[ i ] = v[ i ]; \
	};
#else
#define DEBUG_CODE
#endif

// user defined variables for all equations (to be defined in equation file)
#if ! defined EQ_USER_VARS
#define EQ_USER_VARS
#endif

#define EQ_BEGIN \
	double res = def_res; \
	object *p = var->up, *c = caller, app; \
	int i, j, h, k; \
	double v[1000]; \
	object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10, *cyccur, *cyccur2, *cyccur3; \
	cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cur10 = cyccur = cyccur2 = cyccur3 = NULL; \
	netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5; \
	curl = curl1 = curl2 = curl3 = curl4 = curl5 = NULL; \
	FILE *f = NULL; \
	EQ_USER_VARS

#define EQ_NOT_FOUND \
	char msg[ TCL_BUFF_STR ]; \
	sprintf( msg, "equation not found for variable '%s'\nPossible problems:\n- There is no equation for variable '%s';\n- The spelling in equation's code is different from the name in the configuration;\n- The equation's code was terminated incorrectly", label, label ); \
	error_hard( msg, "Equation not found", "Check your configuration or code to prevent this situation." ); \
	return res;
	
#define EQ_TEST_RESULT \
	if ( quit == 0 && ( ( ! use_nan && is_nan( res ) ) || is_inf( res ) ) ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, "at time %d the equation for '%s' produces the invalid value '%lf',\ncheck the equation code and the temporary values v\\[...\\] to find the faulty line.", t, label, res ); \
		error_hard( msg, "Invalid result", "Check your code to prevent this situation." ); \
		debug_flag = true; \
		debug = 'd'; \
	}

// create and set fast lookup flag
#if ! defined FAST_LOOKUP || ! defined CPP11
	bool fast_lookup = false;
#else
	bool fast_lookup = true;
#endif

// handle fast equation look-up if enabled and C++11 is available
#if ! defined FAST_LOOKUP || ! defined CPP11
	void init_map( ) { };
// use standard chain method for look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		if ( quit == 2 ) \
			return def_res; \
		variable *var = this; \
		EQ_BEGIN
		
#define MODELEND \
		EQ_NOT_FOUND \
		end : \
		EQ_TEST_RESULT \
		DEBUG_CODE \
		return res; \
	}

#define EQUATION( X ) \
	if ( ! strcmp( label, X ) ) { 

#define FUNCTION( X ) \
	if ( ! strcmp( label, X ) ) { \
	last_update--; \
	if ( caller == NULL ) \
	{ \
		res = val[0]; \
		goto end; \
	};

#define RESULT( X ) \
		res = X; \
		goto end; \
	}

#define END_EQUATION( X ) \
	{ \
		res = X; \
		goto end; \
	}

#define DEBUG \
	f = fopen( "log.txt", "a" ); \
	fprintf( f, "t=%d\t%s\t(cur=%g)\n", t, var->label, var->val[0]); \
	fclose( f );
 
#define DEBUG_AT( X ) \
	if ( t >= X ) \
	{ \
		DEBUG \
	};

#else
// use fast map method for equation look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		double res = def_res; \
		if ( quit == 2 ) \
			return res; \
		if ( eq_func == NULL ) \
		{ \
			auto eq_it = eq_map.find( label ); \
			if ( eq_it != eq_map.end( ) ) \
				eq_func = eq_it->second; \
			else \
			{ \
				EQ_NOT_FOUND \
			} \
		} \
		res = ( eq_func )( caller, this ); \
		EQ_TEST_RESULT \
		return res; \
	} \
	void init_map( ) \
	{ \
		eq_map = \
		{

#define MODELEND \
		}; \
	}
			
#define EQUATION( X ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			EQ_BEGIN
		
#define FUNCTION( X ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			EQ_BEGIN \
			var->last_update--; \
			if ( caller == NULL ) \
			{ \
				res = var->val[0]; \
				DEBUG_CODE \
				return res; \
			};
	
#define RESULT( X ) \
			res = X; \
			DEBUG_CODE \
			return res; \
		} \
	},

#define END_EQUATION( X ) \
	{ \
		res = X; \
		DEBUG_CODE \
		return res; \
	}

#endif

#define ABORT quit = 1;
#define CURRENT var->val[ 0 ]
#define PARAMETER var->param = 1;
#define FAST fast = true;
#define OBSERVE fast = false;
#define USE_NAN use_nan = true;
#define NO_NAN use_nan = false;
#define DEFAULT_RESULT( X ) def_res = X;
#define RND_GENERATOR( X ) ran_gen = X;
#define RND_SETSEED( X ) seed = X; init_random( X );
#define SLEEP( X ) msleep( ( unsigned ) X )

#define RND_SEED seed
#define PATH ( ( const char * ) path )
#define CONFIG ( ( const char * ) simul_name )
#define T t
#define LAST_T max_step

// regular logging (disabled in fast mode)
#define LOG( ... ) \
	if ( ! fast ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	}
// priority logging (show even in in fast mode)
#define PLOG( ... ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	}

#define V(X) p->cal(p,(char*)X,0)
#define VL(X,Y) p->cal(p,(char*)X,Y)
#define VS(X,Y) X->cal(X,(char*)Y,0)
#define VLS(X,Y,Z) X->cal(X,(char*)Y,Z)

#define V_CHEAT(X,C) p->cal(C,(char*)X,0)
#define VL_CHEAT(X,Y,C) p->cal(C,(char*)X,Y)
#define VS_CHEAT(X,Y,C) X->cal(C,(char*)Y,0)
#define VLS_CHEAT(X,Y,Z,C) X->cal(C,(char*)Y,Z)

#define SUM(X) p->sum((char*)X,0)
#define SUML(X,Y) p->sum((char*)X,Y)
#define SUMS(X,Y) X->sum((char*)Y,0)
#define SUMLS(X,Y,Z) X->sum((char*)Y,Z)

#define STAT(X) p->stat((char*)X, v)
#define STATS(O,X) O->stat((char*)X, v)

#define WHTAVE(X,W) p->whg_av((char*)W,(char*)X,0)
#define WHTAVEL(X,W,Y) p->whg_av((char*)W,(char*)X,Y)
#define WHTAVES(X,W,Y) X->whg_av((char*)W,(char*)Y,0)
#define WHTAVELS(X,W,Y,Z) X->whg_av((char*)W,(char*)Y,Z)

#define INCRS(Q,X,Y) Q->increment((char*)X,Y)
#define INCR(X,Y) p->increment((char*)X,Y)

#define MULT(X,Y) p->multiply((char*)X,Y)
#define MULTS(Q,X,Y) Q->multiply((char*)X,Y)

#define CYCLE(O,L) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE" ); O != NULL; O = go_brother( O ) )
#define CYCLES(C,O,L) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLES" ); O != NULL; O = go_brother( O ) )

#define CYCLE_SAFE(O,L) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur = go_brother( O ); O != NULL; O = cyccur, \
							  cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFE(O,L) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur2 = go_brother( O ); O != NULL; O = cyccur2, \
							  cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFE(O,L) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur3 = go_brother( O ); O != NULL; O = cyccur3, \
							  cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )
#define CYCLE_SAFES(C,O,L) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur = go_brother( O ); O != NULL; O = cyccur, \
								 cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFES(C,O,L) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur2 = go_brother( O ); O != NULL; O = cyccur2, \
								 cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFES(C,O,L) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur3 = go_brother( O ); O != NULL; O = cyccur3, \
								 cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )

#define MAX(X) p->overall_max((char*)X,0)
#define MAXL(X,Y) p->overall_max((char*)X,Y)
#define MAXS(X,Y) X->overall_max((char*)X,0)
#define MAXLS(O,X,Y) O->overall_max((char*)X,Y)

#define WRITE(X,Y) p->write((char*)X,Y,t)
#define WRITEL(X,Y,Z) p->write((char*)X,Y,Z)
#define WRITELL(X,Y,Z,L) p->write((char*)X,Y,Z,L)
#define WRITES(O,X,Y) O->write((char*)X,Y,t)
#define WRITELS(O,X,Y,Z) O->write((char*)X,Y,Z)
#define WRITELLS(O,X,Y,Z,L) O->write((char*)X,Y,Z,L)

#define RECALC( X ) p->recal( ( char * ) X )
#define RECALCS( O, X ) O->recal( ( char * ) X )

#define SEARCH_CND(X,Y) p->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDL(X,Y,Z) p->search_var_cond((char*)X,Y,Z)
#define SEARCH_CNDS(O,X,Y) O->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDLS(O,X,Y,Z) O->search_var_cond((char*)X,Y,Z)

#define SEARCH(X) p->search((char*)X)
#define SEARCHS(Y,X) Y->search((char*)X)

// Seeds turbo search: O=pointer to container object where searched objects are
//                     X=name of object contained inside the searched objects
//					   Y=total number of objects
#define TSEARCH_INI(X) p->initturbo((char*)X,0)
#define TSEARCHS_INI(O,X) O->initturbo((char*)X,0)
// Versions with knowledge of the total number of objects
#define TSEARCHT_INI(X,Y) p->initturbo((char*)X,Y)
#define TSEARCHTS_INI(O,X,Y) O->initturbo((char*)X,Y)

// Performs turbo search: O, X as in TSEARCHS_INI
//					      Y=total number of objects
//                        Z=position of object X to be searched for
#define TSEARCH(X,Z) p->turbosearch((char*)X,0,Z)
#define TSEARCHS(O,X,Z) O->turbosearch((char*)X,0,Z)
// Versions with knowledge of the total number of objects
#define TSEARCHT(X,Y,Z) p->turbosearch((char*)X,Y,Z)
#define TSEARCHTS(O,X,Y,Z) O->turbosearch((char*)X,Y,Z)

#define SORT(X,Y,Z) p->lsdqsort((char*)X,(char*)Y,(char*)Z)
#define SORTS(O,X,Y,Z) O->lsdqsort((char*)X,(char*)Y,(char*)Z)
#define SORT2(X,Y,L,Z) p->lsdqsort((char*)X,(char*)Y,(char*)L,(char*)Z)
#define SORTS2(O,X,Y,L,Z) O->lsdqsort((char*)X,(char*)Y,(char*)L,(char*)Z)

#define UNIFORM( X, Y ) ((X) + RND*((Y)-(X)))

#define ADDOBJ(X) p->add_n_objects2((char*)X,1)
#define ADDOBJL(X,Y) p->add_n_objects2((char*)X,1,(int)Y)
#define ADDOBJS(O,X) O->add_n_objects2((char*)X,1)
#define ADDOBJLS(O,X,Y) O->add_n_objects2((char*)X,1,(int)Y)

#define ADDOBJ_EX(X,Y) p->add_n_objects2((char*)X,1,Y)
#define ADDOBJL_EX(X,Y,Z) p->add_n_objects2((char*)X,1,Y,(int)Z)
#define ADDOBJS_EX(O,X,Y) O->add_n_objects2((char*)X,1,Y)
#define ADDOBJLS_EX(O,X,Y,Z) O->add_n_objects2((char*)X,1,Y,(int)Z)

#define ADDNOBJ(X,Y) p->add_n_objects2((char*)X,(int)Y)
#define ADDNOBJL(X,Y,Z) p->add_n_objects2((char*)X,(int)Y,(int)Z)
#define ADDNOBJS(O,X,Y) O->add_n_objects2((char*)X,(int)Y)
#define ADDNOBJLS(O,X,Y,Z) O->add_n_objects2((char*)X,(int)Y,(int)Z)

#define ADDNOBJ_EX(X,Y,Z) p->add_n_objects2((char*)X,(int)Y,Z)
#define ADDNOBJL_EX(X,Y,Z,W) p->add_n_objects2((char*)X,(int)Y,Z,(int)W)
#define ADDNOBJS_EX(O,X,Y,Z) O->add_n_objects2((char*)X,(int)Y,Z)
#define ADDNOBJLS_EX(O,X,Y,Z,W) O->add_n_objects2((char*)X,(int)Y,Z,(int)W)

#define DELETE(X) X->delete_obj()

#define RNDDRAW(X,Y) p->draw_rnd((char*)X, (char*)Y, 0)
#define RNDDRAWL(X,Y,Z) p->draw_rnd((char*)X, (char*)Y, Z)
#define RNDDRAWS(Z,X,Y) Z->draw_rnd((char*)X, (char*)Y,0)
#define RNDDRAWLS(O,X,Y,Z) O->draw_rnd((char*)X, (char*)Y, Z)

#define RNDDRAWFAIR(X) p->draw_rnd((char*)X)
#define RNDDRAWFAIRS(Z,X) Z->draw_rnd((char*)X)

#define RNDDRAWTOT(X,Y, T) p->draw_rnd((char*)X, (char*)Y, 0, T)
#define RNDDRAWTOTL(X,Y,Z, T) p->draw_rnd((char*)X, (char*)Y, Z, T)
#define RNDDRAWTOTS(Z,X,Y, T) Z->draw_rnd((char*)X, (char*)Y,0, T)
#define RNDDRAWTOTLS(O,X,Y,Z, T) O->draw_rnd((char*)X, (char*)Y, Z, T)

#define INTERACT(X,Y)  p->interact((char*)X,Y, v)
#define INTERACTS(Z,X,Y) Z->interact((char*)X,Y, v)

// NETWORK MACROS
// create a network using as nodes object label X, located inside object O,
// applying generator Y, number of nodes Z, out-degree W and 
// parameter V
#define NETWORK_INI(X,Y,Z,W,V) (p->init_stub_net((char*)X,(char*)Y,(long)Z,(long)W,V))
#define NETWORKS_INI(O,X,Y,Z,W,V) (O==NULL?0.:O->init_stub_net((char*)X,(char*)Y,(long)Z,(long)W,V))

// delete a network using as nodes object label X, located inside object O
#define NETWORK_DEL(X) (p->delete_net((char*)X)
#define NETWORKS_DEL(O,X) if(O!=NULL) O->delete_net((char*)X);

// read a network in Pajek format from file named Y/Z_xx.net (xx is the current seed) 
// using as nodes object with label X located inside object O
#define NETWORK_LOAD(X,Y,Z) (p->read_file_net((char*)X,(char*)Y,(char*)Z,seed-1,"net"))
#define NETWORKS_LOAD(O,X,Y,Z) (O==NULL?0.:O->read_file_net((char*)X,(char*)Y,(char*)Z,seed-1,"net"))

// save a network in Pajek format to file from the network formed by nodes
// with label X located inside object O with filename Y/Z (file name is Y/Z_xx.net)
#define NETWORK_SAVE(X,Y,Z) (p->write_file_net((char*)X,(char*)Y,(char*)Z,seed-1,false))
#define NETWORKS_SAVE(O,X,Y,Z) (O==NULL?0.:O->write_file_net((char*)X,(char*)Y,(char*)Z,seed-1,false))

// add a network snapshot in Pajek format to file from the network formed by nodes
// with label X located inside object O with filename Y/Z (file name is Y/Z_xx.paj)
#define NETWORK_SNAP(X,Y,Z) (p->write_file_net((char*)X,(char*)Y,(char*)Z,seed-1,true))
#define NETWORKS_SNAP(O,X,Y,Z) (O==NULL?0.:O->write_file_net((char*)X,(char*)Y,(char*)Z,seed-1,true))

// shuffle the nodes of a network composed by objects with label X, contained in O
#define SHUFFLE(X) p->shuffle_nodes_net((char*)X);
#define SHUFFLES(O,X) if(O!=NULL)O->shuffle_nodes_net((char*)X);

// random draw one node from a network composed by objects with label X, contained in O
#define RNDDRAW_NET(X) (p->draw_node_net((char*)X))
#define RNDDRAWS_NET(O,X) (O==NULL?NULL:O->draw_node_net((char*)X))

// get the number of nodes of network based on object X, contained in O
#define STAT_NET(X) p->stats_net((char*)X,v);
#define STATS_NET(O,X) if(O!=NULL)O->stats_net((char*)X,v);

// search node objects X, contained in O, for first occurrence of id=Y
#define SEARCH_NET(X,Y) (p->search_node_net((char*)X,(long)Y))
#define SEARCHS_NET(O,X,Y) (O==NULL?NULL:O->search_node_net((char*)X,(long)Y))

// add a network node to object O, defininig id=X and name=Y
#define ADDNODE(X,Y) (p->add_node_net(X,Y,false))
#define ADDNODES(O,X,Y) (O==NULL?NULL:O->add_node_net(X,Y,false))

// delete the node pointed by O
#define DELETENODE p->delete_node_net( );
#define DELETENODES(O) if(O!=NULL)O->delete_node_net( );

// get the id of the node object O
#define V_NODEID (p->node==NULL?0.:(double)p->node->id)
#define VS_NODEID(O) (O==NULL?0.:O->node==NULL?0.:(double)O->node->id)

// get the name of the node object O
#define V_NODENAME (p->node==NULL?"":p->node->name==NULL?"":p->node->name)
#define VS_NODENAME(O) (O==NULL?"":O->node==NULL?"":O->node->name==NULL?"":O->node->name)

// set the id of the node object O
#define WRITE_NODEID(X) if(p->node!=NULL)p->node->id=(double)X;
#define WRITES_NODEID(O,X) if(O!=NULL)if(O->node!=NULL)O->node->id=(double)X;

// set the name of the node object O to X
#define WRITE_NODENAME(X) p->name_node_net((char*)X);
#define WRITES_NODENAME(O,X) if(O!=NULL)O->name_node_net((char*)X);

// get the number of outgoing links from object O
#define STAT_NODE p->node==NULL?v[0]=0.:v[0]=(double)p->node->nLinks;
#define STATS_NODE(O) O==NULL?v[0]=0.:O->node==NULL?v[0]=0.:v[0]=(double)O->node->nLinks;

// add a link from object O to object X, both located inside same parent, same label
// and optional weight Y
#define ADDLINK(X) (p->add_link_net(X,0,1))
#define ADDLINKS(O,X) (O==NULL?NULL:O->add_link_net(X,0,1))
#define ADDLINKW(X,Y) (p->add_link_net(X,Y,1))
#define ADDLINKWS(O,X,Y) (O==NULL?NULL:O->add_link_net(X,Y,1))

// delete the link pointed by O
#define DELETELINK(O) if(O!=NULL)O->ptrFrom->delete_link_net(O);

// search outgoing links from object O for first occurrence of id=X
#define SEARCH_LINK(X) (p->search_link_net((long)X))
#define SEARCHS_LINK(O,X) (O==NULL?NULL:O->search_link_net((long)X))

// random draw one link from a node
#define RNDDRAW_LINK (p->draw_link_net())
#define RNDDRAWS_LINK(O) (O==NULL?NULL:O->draw_link_net())

// get the destination object of link pointed by O
#define LINKTO(O) (O==NULL?NULL:O->ptrTo)

// get the destination object of link pointed by O
#define LINKFROM(O) (O==NULL?NULL:O->ptrFrom)

// get the weight of link pointed by O
#define VS_WEIGHT(O) (O==NULL?0.:O->weight)

// set the weight of link pointed by O to X
#define WRITES_WEIGHT(O,X) if(O!=NULL)O->weight=X;

// cycle through set of links of object C, using link pointer O
#define CYCLE_LINK(O) if ( p->node == NULL || p->node->first == NULL ) \
	error_hard( "object has incorrect network data structure", "Network data structure missing", "Check your code to prevent this situation." ); \
else \
	for ( O = p->node->first; O != NULL; O = O->next )

#define CYCLES_LINK(C,O) if ( C == NULL || C->node == NULL || C->node->first == NULL ) \
	error_hard( "object has incorrect network data structure", "Network data structure missing", "Check your code to prevent this situation." ); \
else \
	for ( O = C->node->first; O != NULL; O = O->next )


// EXTENDED DATA/ATTRIBUTES MANAGEMENT MACROS
// macros for handling extended attributes (usually lists) attached to objects' cext pointer

// add/delete extension c++ data structures of type CLASS to a void pointer by current/PTR
#define ADD_EXT( CLASS ) p->cext = reinterpret_cast< void * >( new CLASS );
#define ADDS_EXT( PTR, CLASS ) PTR->cext = reinterpret_cast< void * >( new CLASS );
#define DELETE_EXT( CLASS ) { delete reinterpret_cast< CLASS * >( p->cext ); p->cext = NULL; }
#define DELETES_EXT( PTR, CLASS ) { delete reinterpret_cast< CLASS * >( PTR->cext ); PTR->cext = NULL; }

// convert current (or a pointer PTR from) void type in the user defined CLASS type
#define P_EXT( CLASS ) ( reinterpret_cast< CLASS * >( p->cext ) )
#define PS_EXT( PTR, CLASS ) ( reinterpret_cast< CLASS * >( PTR->cext ) )

// read/write from object OBJ pointed by pointer current/PTR of type CLASS
#define V_EXT( CLASS, OBJ ) ( P_EXT( CLASS ) -> OBJ )
#define VS_EXT( PTR, CLASS, OBJ ) ( PS_EXT( PTR, CLASS ) -> OBJ )
#define WRITE_EXT( CLASS, OBJ, VAL ) ( P_EXT( CLASS ) -> OBJ = VAL )
#define WRITES_EXT( PTR, CLASS, OBJ, VAL ) ( PS_EXT( PTR, CLASS ) -> OBJ = VAL )

// execute METHOD contained in OBJ pointed by pointer current/PTR of type CLASS with the parameters ...
#define EXEC_EXT( CLASS, OBJ, METHOD, ... ) ( P_EXT( CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )
#define EXECS_EXT( PTR, CLASS, OBJ, METHOD, ... ) ( PS_EXT( PTR, CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )

// cycle over elements of OBJ pointed by pointer current/PTR of type CLASS using iterator ITER
#define CYCLE_EXT( ITER, CLASS, OBJ ) for ( ITER = EXEC_EXT( CLASS, OBJ, begin ); ITER != EXEC_EXT( CLASS, OBJ, end ); ++ITER )
#define CYCLES_EXT( PTR, ITER, CLASS, OBJ ) for ( ITER = EXECS_EXT( PTR, CLASS, OBJ, begin ); ITER != EXECS_EXT( PTR, CLASS, OBJ, end ); ++ITER )
