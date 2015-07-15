/**\file */
#ifndef SLIC_DECLARATIONS_RayTracer_H
#define SLIC_DECLARATIONS_RayTracer_H
#include "MaxSLiCInterface.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RayTracer_RaysWordWidthInBits (384)
#define RayTracer_RaysPerTick (2)
#define RayTracer_TriangleWidthInBytes (36)
#define RayTracer_RaysPerWord (2)
#define RayTracer_TrianglesInWidthInBits (3072)
#define RayTracer_PCIE_ALIGNMENT (16)
#define RayTracer_TrianglesPerTick (10)


/*----------------------------------------------------------------------------*/
/*---------------------- Interface memoryInitialisation ----------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'memoryInitialisation'.
 * 
 * \param [in] param_address Interface Parameter "address".
 * \param [in] param_size Interface Parameter "size".
 * \param [in] instream_triangles_in The stream should be of size param_size bytes.
 */
void RayTracer_memoryInitialisation(
	int32_t param_address,
	int32_t param_size,
	const int32_t *instream_triangles_in);

/**
 * \brief Basic static non-blocking function for the interface 'memoryInitialisation'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [in] param_address Interface Parameter "address".
 * \param [in] param_size Interface Parameter "size".
 * \param [in] instream_triangles_in The stream should be of size param_size bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *RayTracer_memoryInitialisation_nonblock(
	int32_t param_address,
	int32_t param_size,
	const int32_t *instream_triangles_in);

/**
 * \brief Advanced static interface, structure for the engine interface 'memoryInitialisation'
 * 
 */
typedef struct { 
	int32_t param_address; /**<  [in] Interface Parameter "address". */
	int32_t param_size; /**<  [in] Interface Parameter "size". */
	const int32_t *instream_triangles_in; /**<  [in] The stream should be of size param_size bytes. */
} RayTracer_memoryInitialisation_actions_t;

/**
 * \brief Advanced static function for the interface 'memoryInitialisation'.
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in,out] interface_actions Actions to be executed.
 */
void RayTracer_memoryInitialisation_run(
	max_engine_t *engine,
	RayTracer_memoryInitialisation_actions_t *interface_actions);

/**
 * \brief Advanced static non-blocking function for the interface 'memoryInitialisation'.
 *
 * Schedule the actions to run on the engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in] interface_actions Actions to be executed.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_memoryInitialisation_run_nonblock(
	max_engine_t *engine,
	RayTracer_memoryInitialisation_actions_t *interface_actions);

/**
 * \brief Group run advanced static function for the interface 'memoryInitialisation'.
 * 
 * \param [in] group Group to use.
 * \param [in,out] interface_actions Actions to run.
 *
 * Run the actions on the first device available in the group.
 */
void RayTracer_memoryInitialisation_run_group(max_group_t *group, RayTracer_memoryInitialisation_actions_t *interface_actions);

/**
 * \brief Group run advanced static non-blocking function for the interface 'memoryInitialisation'.
 * 
 *
 * Schedule the actions to run on the first device available in the group and return immediately.
 * The status of the run must be checked with ::max_wait. 
 * Note that use of ::max_nowait is prohibited with non-blocking running on groups:
 * see the ::max_run_group_nonblock documentation for more explanation.
 *
 * \param [in] group Group to use.
 * \param [in] interface_actions Actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_memoryInitialisation_run_group_nonblock(max_group_t *group, RayTracer_memoryInitialisation_actions_t *interface_actions);

/**
 * \brief Array run advanced static function for the interface 'memoryInitialisation'.
 * 
 * \param [in] engarray The array of devices to use.
 * \param [in,out] interface_actions The array of actions to run.
 *
 * Run the array of actions on the array of engines.  The length of interface_actions
 * must match the size of engarray.
 */
void RayTracer_memoryInitialisation_run_array(max_engarray_t *engarray, RayTracer_memoryInitialisation_actions_t *interface_actions[]);

/**
 * \brief Array run advanced static non-blocking function for the interface 'memoryInitialisation'.
 * 
 *
 * Schedule to run the array of actions on the array of engines, and return immediately.
 * The length of interface_actions must match the size of engarray.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * \param [in] engarray The array of devices to use.
 * \param [in] interface_actions The array of actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_memoryInitialisation_run_array_nonblock(max_engarray_t *engarray, RayTracer_memoryInitialisation_actions_t *interface_actions[]);

/**
 * \brief Converts a static-interface action struct into a dynamic-interface max_actions_t struct.
 *
 * Note that this is an internal utility function used by other functions in the static interface.
 *
 * \param [in] maxfile The maxfile to use.
 * \param [in] interface_actions The interface-specific actions to run.
 * \return The dynamic-interface actions to run, or NULL in case of error.
 */
max_actions_t* RayTracer_memoryInitialisation_convert(max_file_t *maxfile, RayTracer_memoryInitialisation_actions_t *interface_actions);



/*----------------------------------------------------------------------------*/
/*---------------------------- Interface default -----------------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'default'.
 * 
 * \param [in] ticks_MemoryCommandGenerator The number of ticks for which kernel "MemoryCommandGenerator" will run.
 * \param [in] ticks_RayTracerKernel The number of ticks for which kernel "RayTracerKernel" will run.
 * \param [in] inscalar_MemoryCommandGenerator_triangles_to_read_in_bursts Input scalar parameter "MemoryCommandGenerator.triangles_to_read_in_bursts".
 * \param [in] inscalar_RayTracerKernel_total_rays Input scalar parameter "RayTracerKernel.total_rays".
 * \param [in] inscalar_RayTracerKernel_total_triangles Input scalar parameter "RayTracerKernel.total_triangles".
 * \param [in] instream_rays_in Stream "rays_in".
 * \param [in] instream_size_rays_in The size of the stream instream_rays_in in bytes.
 * \param [in] instream_triangles_in Stream "triangles_in".
 * \param [in] instream_size_triangles_in The size of the stream instream_triangles_in in bytes.
 * \param [out] outstream_results_out Stream "results_out".
 * \param [in] outstream_size_results_out The size of the stream outstream_results_out in bytes.
 * \param [out] outstream_status_out Stream "status_out".
 * \param [in] outstream_size_status_out The size of the stream outstream_status_out in bytes.
 * \param [in] lmem_address_triangles_to_mem Linear LMem control for "triangles_to_mem" stream: base address, in bytes.
 * \param [in] lmem_arr_size_triangles_to_mem Linear LMem control for "triangles_to_mem" stream: array size, in bytes.
 */
void RayTracer(
	uint64_t ticks_MemoryCommandGenerator,
	uint64_t ticks_RayTracerKernel,
	uint64_t inscalar_MemoryCommandGenerator_triangles_to_read_in_bursts,
	uint64_t inscalar_RayTracerKernel_total_rays,
	uint64_t inscalar_RayTracerKernel_total_triangles,
	const void *instream_rays_in,
	size_t instream_size_rays_in,
	const void *instream_triangles_in,
	size_t instream_size_triangles_in,
	void *outstream_results_out,
	size_t outstream_size_results_out,
	void *outstream_status_out,
	size_t outstream_size_status_out,
	size_t lmem_address_triangles_to_mem,
	size_t lmem_arr_size_triangles_to_mem);

/**
 * \brief Basic static non-blocking function for the interface 'default'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [in] ticks_MemoryCommandGenerator The number of ticks for which kernel "MemoryCommandGenerator" will run.
 * \param [in] ticks_RayTracerKernel The number of ticks for which kernel "RayTracerKernel" will run.
 * \param [in] inscalar_MemoryCommandGenerator_triangles_to_read_in_bursts Input scalar parameter "MemoryCommandGenerator.triangles_to_read_in_bursts".
 * \param [in] inscalar_RayTracerKernel_total_rays Input scalar parameter "RayTracerKernel.total_rays".
 * \param [in] inscalar_RayTracerKernel_total_triangles Input scalar parameter "RayTracerKernel.total_triangles".
 * \param [in] instream_rays_in Stream "rays_in".
 * \param [in] instream_size_rays_in The size of the stream instream_rays_in in bytes.
 * \param [in] instream_triangles_in Stream "triangles_in".
 * \param [in] instream_size_triangles_in The size of the stream instream_triangles_in in bytes.
 * \param [out] outstream_results_out Stream "results_out".
 * \param [in] outstream_size_results_out The size of the stream outstream_results_out in bytes.
 * \param [out] outstream_status_out Stream "status_out".
 * \param [in] outstream_size_status_out The size of the stream outstream_status_out in bytes.
 * \param [in] lmem_address_triangles_to_mem Linear LMem control for "triangles_to_mem" stream: base address, in bytes.
 * \param [in] lmem_arr_size_triangles_to_mem Linear LMem control for "triangles_to_mem" stream: array size, in bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *RayTracer_nonblock(
	uint64_t ticks_MemoryCommandGenerator,
	uint64_t ticks_RayTracerKernel,
	uint64_t inscalar_MemoryCommandGenerator_triangles_to_read_in_bursts,
	uint64_t inscalar_RayTracerKernel_total_rays,
	uint64_t inscalar_RayTracerKernel_total_triangles,
	const void *instream_rays_in,
	size_t instream_size_rays_in,
	const void *instream_triangles_in,
	size_t instream_size_triangles_in,
	void *outstream_results_out,
	size_t outstream_size_results_out,
	void *outstream_status_out,
	size_t outstream_size_status_out,
	size_t lmem_address_triangles_to_mem,
	size_t lmem_arr_size_triangles_to_mem);

/**
 * \brief Advanced static interface, structure for the engine interface 'default'
 * 
 */
typedef struct { 
	uint64_t ticks_MemoryCommandGenerator; /**<  [in] The number of ticks for which kernel "MemoryCommandGenerator" will run. */
	uint64_t ticks_RayTracerKernel; /**<  [in] The number of ticks for which kernel "RayTracerKernel" will run. */
	uint64_t inscalar_MemoryCommandGenerator_triangles_to_read_in_bursts; /**<  [in] Input scalar parameter "MemoryCommandGenerator.triangles_to_read_in_bursts". */
	uint64_t inscalar_RayTracerKernel_total_rays; /**<  [in] Input scalar parameter "RayTracerKernel.total_rays". */
	uint64_t inscalar_RayTracerKernel_total_triangles; /**<  [in] Input scalar parameter "RayTracerKernel.total_triangles". */
	const void *instream_rays_in; /**<  [in] Stream "rays_in". */
	size_t instream_size_rays_in; /**<  [in] The size of the stream instream_rays_in in bytes. */
	const void *instream_triangles_in; /**<  [in] Stream "triangles_in". */
	size_t instream_size_triangles_in; /**<  [in] The size of the stream instream_triangles_in in bytes. */
	void *outstream_results_out; /**<  [out] Stream "results_out". */
	size_t outstream_size_results_out; /**<  [in] The size of the stream outstream_results_out in bytes. */
	void *outstream_status_out; /**<  [out] Stream "status_out". */
	size_t outstream_size_status_out; /**<  [in] The size of the stream outstream_status_out in bytes. */
	size_t lmem_address_triangles_to_mem; /**<  [in] Linear LMem control for "triangles_to_mem" stream: base address, in bytes. */
	size_t lmem_arr_size_triangles_to_mem; /**<  [in] Linear LMem control for "triangles_to_mem" stream: array size, in bytes. */
} RayTracer_actions_t;

/**
 * \brief Advanced static function for the interface 'default'.
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in,out] interface_actions Actions to be executed.
 */
void RayTracer_run(
	max_engine_t *engine,
	RayTracer_actions_t *interface_actions);

/**
 * \brief Advanced static non-blocking function for the interface 'default'.
 *
 * Schedule the actions to run on the engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in] interface_actions Actions to be executed.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_run_nonblock(
	max_engine_t *engine,
	RayTracer_actions_t *interface_actions);

/**
 * \brief Group run advanced static function for the interface 'default'.
 * 
 * \param [in] group Group to use.
 * \param [in,out] interface_actions Actions to run.
 *
 * Run the actions on the first device available in the group.
 */
void RayTracer_run_group(max_group_t *group, RayTracer_actions_t *interface_actions);

/**
 * \brief Group run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule the actions to run on the first device available in the group and return immediately.
 * The status of the run must be checked with ::max_wait. 
 * Note that use of ::max_nowait is prohibited with non-blocking running on groups:
 * see the ::max_run_group_nonblock documentation for more explanation.
 *
 * \param [in] group Group to use.
 * \param [in] interface_actions Actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_run_group_nonblock(max_group_t *group, RayTracer_actions_t *interface_actions);

/**
 * \brief Array run advanced static function for the interface 'default'.
 * 
 * \param [in] engarray The array of devices to use.
 * \param [in,out] interface_actions The array of actions to run.
 *
 * Run the array of actions on the array of engines.  The length of interface_actions
 * must match the size of engarray.
 */
void RayTracer_run_array(max_engarray_t *engarray, RayTracer_actions_t *interface_actions[]);

/**
 * \brief Array run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule to run the array of actions on the array of engines, and return immediately.
 * The length of interface_actions must match the size of engarray.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * \param [in] engarray The array of devices to use.
 * \param [in] interface_actions The array of actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *RayTracer_run_array_nonblock(max_engarray_t *engarray, RayTracer_actions_t *interface_actions[]);

/**
 * \brief Converts a static-interface action struct into a dynamic-interface max_actions_t struct.
 *
 * Note that this is an internal utility function used by other functions in the static interface.
 *
 * \param [in] maxfile The maxfile to use.
 * \param [in] interface_actions The interface-specific actions to run.
 * \return The dynamic-interface actions to run, or NULL in case of error.
 */
max_actions_t* RayTracer_convert(max_file_t *maxfile, RayTracer_actions_t *interface_actions);

/**
 * \brief Initialise a maxfile.
 */
max_file_t* RayTracer_init(void);

/* Error handling functions */
int RayTracer_has_errors(void);
const char* RayTracer_get_errors(void);
void RayTracer_clear_errors(void);
/* Free statically allocated maxfile data */
void RayTracer_free(void);
/* returns: -1 = error running command; 0 = no error reported */
int RayTracer_simulator_start(void);
/* returns: -1 = error running command; 0 = no error reported */
int RayTracer_simulator_stop(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SLIC_DECLARATIONS_RayTracer_H */

