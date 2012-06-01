/**
	@file
	HoaEncode - High Order Ambisonic Panner
	Julien Colafrancesco	

	@ingroup	examples	
*/

extern "C" {
#include "ext.h"							// standard Max include, always required (except in Jitter)
#include "ext_obex.h"						// required for new style objects
#include "z_dsp.h"							// required for MSP objects
}

#include <AmbisonicEncode.h>

////////////////////////// object struct
typedef struct _HoaEncode 
{
	t_pxobject					ob;			
	AmbisonicEncode*			AmbiEncoder;
	t_float**					sp_vec_copy;
	int							sp_n_copy;
	long						inputNumber;
	long						outputNumber;

} t_HoaEncode;

///////////////////////// function prototypes
//// standard set
void *HoaEncode_new(t_symbol *s, long argc, t_atom *argv);
void HoaEncode_free(t_HoaEncode *x);
void HoaEncode_assist(t_HoaEncode *x, void *b, long m, long a, char *s);

void HoaEncode_float(t_HoaEncode *x, double f);

void HoaEncode_dsp(t_HoaEncode *x, t_signal **sp, short *count);
t_int *HoaEncode_perform(t_int *w);
//////////////////////// global class pointer variable
void *HoaEncode_class;
    

int main(void)
{	

	t_class *c;
	
	c = class_new("Hoa.Encoder~", (method)HoaEncode_new, (method)HoaEncode_free, (long)sizeof(t_HoaEncode), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)HoaEncode_float,		"float",	A_FLOAT, 0);
	class_addmethod(c, (method)HoaEncode_dsp,		"dsp",		A_CANT, 0);
	class_addmethod(c, (method)HoaEncode_assist,	"assist",	A_CANT, 0);
	
	class_dspinit(c);				
	class_register(CLASS_BOX, c);	
	HoaEncode_class = c;
	
	return 0;
}

void HoaEncode_float(t_HoaEncode *x, double f)
{
	x->AmbiEncoder->computeCoefs(f);
}


void HoaEncode_dsp(t_HoaEncode *x, t_signal **sp, short *count)
{
	int outputNumber = x->outputNumber;
	int inputNumber = x->inputNumber;
	
	for (int i = 0; i<outputNumber+inputNumber; i++) {
		x->sp_vec_copy[i] = sp[i]->s_vec;
	}
	x->sp_n_copy = (int)sp[0]->s_n;
	
	dsp_add(HoaEncode_perform, 1, x);
}

t_int *HoaEncode_perform(t_int *w)
{
	t_HoaEncode *x = (t_HoaEncode *)(w[1]);

	int n;
	int outputNumber = x->outputNumber;
	int inputNumber = x->inputNumber;

	t_float *in;
	t_float *out;
	
	double* tmp_in_vector;
	tmp_in_vector = new double[inputNumber];

	n = x->sp_n_copy;
	std::vector<double> ComputedOuput;
	
	in = (t_float*)x->sp_vec_copy[0];
	
	for (int i = 0; i<n; i++) {
		
		for (int k = 0; k < inputNumber; k++) {
			in = (t_float*)x->sp_vec_copy[k];
			tmp_in_vector[k] =  in[i];
		}
		
		ComputedOuput = x->AmbiEncoder->process(tmp_in_vector);
		
		for (int j = 0; j<outputNumber; j++) {
			out = (t_float*)x->sp_vec_copy[inputNumber+j];
			out[i] = ComputedOuput[j];
		}
	}
	return w + 2;
}


void HoaEncode_assist(t_HoaEncode *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "Inlet %ld", a);
	} 
	else {	// outlet
		sprintf(s, "Outlet %ld", a); 			
	}
}


void HoaEncode_free(t_HoaEncode *x) 
{

	dsp_free((t_pxobject *)x);
	sysmem_freeptr(x->sp_vec_copy);

	
	
}

void *HoaEncode_new(t_symbol *s, long argc, t_atom *argv)
{
	t_HoaEncode *x = NULL;

	long orderNumber = atom_getlong(argv);
	long outputNumber = 2*orderNumber+1;
	long inputNumber = orderNumber+1;
	

	
	
	if (x = (t_HoaEncode *)object_alloc((t_class*)HoaEncode_class)) {
		dsp_setup((t_pxobject *)x, inputNumber);	
		
		for (int i=0; i<outputNumber; i++) {
			outlet_new(x, "signal");
		}

		x->AmbiEncoder = new AmbisonicEncode(orderNumber);
		//x->sp_vec_copy = new t_float* [outputNumber+inputNumber];
		
		x->sp_vec_copy = (t_float **)sysmem_newptr (sizeof(t_float*) * (outputNumber+inputNumber));
		x->ob.z_misc = Z_NO_INPLACE;  
		x->inputNumber = inputNumber;
		x->outputNumber = outputNumber;

		
	}
	return (x);
}
