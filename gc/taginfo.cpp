/************************************************************************************//**
		* File: taginfo.cpp
		* Description: realisation of functions from 'taginfo.h'
*****************************************************************************************/

#include "taginfo.h"
#include <sys/mman.h>
#include <assert.h>
#include "debug_print.h"

/** 
* @brief generation box for struct
* @detailed  create the pointer on object with create_generic_object function
* @param offsets_ptr - list of pointer offsets in struct, size - full size of real struct, num_el - num of this type elements!!!! ?num of pointers 
* @return the pointer on object
*/
void * generic_box_struct (std::vector <size_t> offsets_ptr, size_t size, size_t num_el) {
	void* object; /**< a stored pointer */
	try {
		/*
		 * call function creating box object for struct > 
		 * first arg - num of offsets, sec arg - size of real struct, thd arg - num of pointers in the struct>
		 */
		object = create_generic_object(offsets_ptr.size(), size, num_el);
	#ifdef DEBUGE_MODE
		printf("gen_boc_struct: %zu offsets : ", offsets_ptr.size());
		for(int i = 0; i < offsets_ptr.size(); i++) {
			printf("%zu| ", offsets_ptr[i]);
		}
	#endif
		std::vector <size_t>::iterator it_offset = offsets_ptr.begin(); /**< create iterator for offsets_ptr*/
		POINTER_DESCR descr; /**< temprorary element for saving offset*/
		for ( size_t iter_p = 0; iter_p < offsets_ptr.size(); iter_p++, it_offset++) { /* save all pointers in object */
			descr = {*it_offset, 0}; /* save pointers in descriptor */
			dprintf("%zu ", *it_offset);
			/*
			 * call function write descriptor in object 
			 * first arg - object reference, sec arg - index place in object, thd arg - descriptor
			 */
			set_ptr_descr(object, iter_p, descr); 
		}
		dprintf("\n");
	}
	catch (...) {
		printf("Error! Couldn't create generic box struct!");
	}
	return object;
}

/* simple, 1-word object with num 2, struct with num 1*/
void * create_generic_object (size_t descr_length, size_t size, size_t num_of_el) {
	void  * result = NULL;
	try {	
		if (descr_length == 0) {
			BLOCK_TAG tag = {TAG_MODEL_2, size, num_of_el, NULL};
			result = mmap(0, sizeof (BLOCK_TAG) + 1, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			assert (result != MAP_FAILED);
			dprintf("create_generic_object descr_length == 0 %p\n", result);
			*(BLOCK_TAG *)result = tag;
		} else {
			BLOCK_TAG tag = {TAG_MODEL_1, size, num_of_el, NULL};
			result = mmap(0, sizeof (BLOCK_TAG) + sizeof(size_t) + descr_length * sizeof(POINTER_DESCR),
				PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			assert (result != MAP_FAILED);
			dprintf("create_generic_object descr_length != 0 %p\ndescr_length: %zu\n", result, descr_length);
			*(BLOCK_TAG *)result = tag;
			*(size_t*)((char *)result + sizeof(BLOCK_TAG)) = descr_length;
		}
	} catch (...) {
		printf("UNEXPECTED ERROR! Function create_generic_object.");
		fflush(stdout);
	}
	return result;
}

/* setting descriptors*/
void set_ptr_descr (void* object, unsigned char iter_p, POINTER_DESCR descr) {
	try {
		*((POINTER_DESCR*)((char *)object + sizeof(BLOCK_TAG) + sizeof(size_t) + sizeof(POINTER_DESCR) * (size_t)(iter_p))) = descr;
	} catch (...) {
		printf("UNEXPECTED ERROR! Couldn't set descriptor in object.");
		fflush(stdout);
	}		
	return;
}

/* array of boxed objects*/
void * create_boxed_array(size_t size, void * clMeta, size_t typeSize) {
	void * result = NULL;	
	try {
		BLOCK_TAG tag = {TAG_MODEL_3, size, typeSize, clMeta};
		result = mmap(0, sizeof (BLOCK_TAG),
			PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert (result != MAP_FAILED);
		dprintf("create_boxed_array %p\n", result);
		*(BLOCK_TAG *)result = tag;
	} catch (...) {
		printf("UNEXPECTED ERROR! Function create_boxed_array.");
		fflush(stdout);	
	}
	return result;
}

/* array of unboxed objects*/
void * create_unboxed_array(size_t size) {
	void * result = NULL;	
	try{
		BLOCK_TAG tag = {TAG_MODEL_4, size, 0, NULL};
		result = mmap(0, size * sizeof (word_t) + sizeof (BLOCK_TAG) + 1,
			PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert (result != MAP_FAILED);
		dprintf("create_unboxed_array %p\n", result);
		*(BLOCK_TAG *)result = tag;
	} catch(...) {
		printf("UNEXPECTED ERROR! Function create_unboxed_array.");
		fflush(stdout);
	}
	return result;
}

PTR_ITERATOR get_iterator (void * object) {
	PTR_ITERATOR res_ptr;
	char * begin_arr;
	unsigned int i = 0;
	char * ptr_obj =  (char *) object;
	try {
		switch (((BLOCK_TAG *) object)->model) {
		case TAG_MODEL_1:
			res_ptr.current = ((POINTER_DESCR *)((char *)ptr_obj + sizeof (BLOCK_TAG) + 1))->offset; 
			res_ptr.object = object;
			break;
		case TAG_MODEL_2:
			res_ptr.object = NULL;  
		case TAG_MODEL_3:
			res_ptr.current = 0;
			res_ptr.object = object; 
			break;
		case TAG_MODEL_4:
			res_ptr.object = NULL;  
			break;
		}
	}
	catch(...) {
		printf("UNEXPECTED ERROR! Function get_iterator.");
		fflush(stdout);
	}
		return res_ptr;
}

void * get_ptr (void  * object, size_t index) {
	int bitmapsize;
	char descr_length;
	char * ptr_obj =  (char *) object;
	try {
		switch (((BLOCK_TAG *) object)->model) {
		case TAG_MODEL_1:
			descr_length = *((char *)object + sizeof(BLOCK_TAG));
			return (ptr_obj + (index *sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR))); 
			break;
		case TAG_MODEL_2:
			return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
			break;
		case TAG_MODEL_3:
			return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
			break;
		case TAG_MODEL_4:
			return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
			break;
		default:
			throw;
			return 0;
			break;
		}
	} catch(...) {
		printf("UNEXPECTED ERROR! Function get_ptr.");
		fflush(stdout);
	}
}

void * next_ptr (PTR_ITERATOR * iterator) {
	void * res_ptr;
	unsigned int i = 0;
	char * begin_arr;	
	try {
		switch (((BLOCK_TAG *)(iterator->object))->model) {
		case TAG_MODEL_1:
			res_ptr = get_ptr(iterator->object,iterator->current);
			while(i < ((BLOCK_TAG *)(iterator->object))->size && iterator->current != ((POINTER_DESCR *)((char *)(iterator->object) + sizeof (BLOCK_TAG) + 1 + i * sizeof(POINTER_DESCR)))->offset) {
				i++;
			}
			if(i == ((BLOCK_TAG *)(iterator->object))->size) {
				iterator->object = NULL;
			}
			iterator->current = ((POINTER_DESCR *)((char *)(iterator->object) + sizeof (BLOCK_TAG) + 1 + i * sizeof(POINTER_DESCR)))->offset;
			break;
		case TAG_MODEL_2:
			iterator->object = NULL; 
		case TAG_MODEL_3:
			res_ptr = get_ptr(iterator->object,iterator->current);
			iterator->current++;
			break;
		case TAG_MODEL_4:
			iterator->object = NULL; 
			break;
		default:
			throw;
			return 0;
			break;
		}
	} catch(...) {
		printf("UNEXPECTED ERROR! Function next_ptr.");
		fflush(stdout);
	}
	return res_ptr;
}
