/**
	@file
	HoaDecode - High Order Ambisonic Panner
	Julien Colafrancesco	

	@ingroup	examples	
*/
extern "C" {
#include "ext.h"							// standard Max include, always required (except in Jitter)
#include "ext_obex.h"						// required for new style objects
#include "z_dsp.h"							// required for MSP objects
}

#include <ambisonicDecode.h>
////////////////////////// object struct
typedef struct _HoaDecode 
{
	t_pxobject					ob;			
	ambisonicDecode*			ambisonicDecoder;
	t_float**					sp_vec_copy;
	int							sp_n_copy;
	long						inputNumber;
	long						outputNumber;

} t_HoaDecode;

///////////////////////// function prototypes
//// standard set
void *HoaDecode_new(t_symbol *s, long argc, t_atom *argv);
void HoaDecode_free(t_HoaDecode *x);
void HoaDecode_assist(t_HoaDecode *x, void *b, long m, long a, char *s);

void HoaDecode_float(t_HoaDecode *x, double f);

void HoaDecode_dsp(t_HoaDecode *x, t_signal **sp, short *count);
t_int *HoaDecode_perform(t_int *w);
//////////////////////// global class pointer variable
void *HoaDecode_class;


int main(void)
{	

	t_class *c;
	
	c = class_new("Hoa.Decoder~", (method)HoaDecode_new, (method)HoaDecode_free, (long)sizeof(t_HoaDecode), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)HoaDecode_float,		"float",	A_FLOAT, 0);
	class_addmethod(c, (method)HoaDecode_dsp,		"dsp",		A_CANT, 0);
	class_addmethod(c, (method)HoaDecode_assist,	"assist",	A_CANT, 0);
	
	class_dspinit(c);				
	class_register(CLASS_BOX, c);	
	HoaDecode_class = c;
	
	return 0;
}

void HoaDecode_float(t_HoaDecode *x, double f)
{

}


void HoaDecode_dsp(t_HoaDecode *x, t_signal **sp, short *count)
{
	int outputNumber = x->outputNumber;
	int inputNumber = x->inputNumber;
	
	for (int i = 0; i<outputNumber+inputNumber; i++) {
		x->sp_vec_copy[i] = sp[i]->s_vec;
	}
	x->sp_n_copy = (int)sp[0]->s_n;
	
	dsp_add(HoaDecode_perform, 1, x);
}

t_int *HoaDecode_perform(t_int *w)
{
	t_HoaDecode *x = (t_HoaDecode *)(w[1]);
	int n;
	int channelNumber = x->outputNumber;
	int inputNumber = x->inputNumber;
	
	t_float *in;
	t_float *out;
	
	double* tmp_in_vector;
	tmp_in_vector = new double[inputNumber];
	
	std::vector<double>* tmp_out_vector;
	
	n = x->sp_n_copy;
	
	
	for (int i = 0; i<n; i++) 
	{


		for (int k = 0; k < inputNumber; k++) {
			in = (t_float*)x->sp_vec_copy[k];
			tmp_in_vector[k] =  in[i];
		}
		
		tmp_out_vector = x->ambisonicDecoder->process(tmp_in_vector);
		
		for (int j = 0; j<channelNumber; j++) 
		{
			out = (t_float*)x->sp_vec_copy[inputNumber+j];
			out[i] = (*tmp_out_vector)[j];
		}
	}
	return w + 2;
}

void HoaDecode_assist(t_HoaDecode *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "Inlet %ld", a);
	} 
	else {	// outlet
		sprintf(s, "Outlet %ld", a); 			
	}
}


void HoaDecode_free(t_HoaDecode *x) 
{

	dsp_free((t_pxobject *)x);
	sysmem_freeptr(x->sp_vec_copy);

	
	
}

void *HoaDecode_new(t_symbol *s, long argc, t_atom *argv)
{
	t_HoaDecode *x = NULL;

	long orderNumber = atom_getlong(argv);
	long outputNumber = atom_getlong(argv+1);
	long inputNumber = orderNumber*2+1;
	

	
	
	if (x = (t_HoaDecode *)object_alloc((t_class*)HoaDecode_class)) {
		dsp_setup((t_pxobject *)x, inputNumber);	
		
		for (int i=0; i<outputNumber; i++) {
			outlet_new(x, "signal");
		}

		x->ambisonicDecoder = new ambisonicDecode(outputNumber, orderNumber);
		//x->sp_vec_copy = new t_float* [outputNumber+inputNumber];
		
		x->sp_vec_copy = (t_float **)sysmem_newptr (sizeof(t_float*) * (outputNumber+inputNumber));
		x->ob.z_misc = Z_NO_INPLACE;  
		x->inputNumber = inputNumber;
		x->outputNumber = outputNumber;

		
	}
	return (x);
}
