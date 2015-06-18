/**\file */
#ifndef SLIC_DECLARATIONS_RayTracer_H
#define SLIC_DECLARATIONS_RayTracer_H
#include "MaxSLiCInterface.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RayTracer_PCIE_ALIGNMENT (16)


/*----------------------------------------------------------------------------*/
/*---------------------------- Interface default -----------------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'default'.
 * 
 * \param [in] ticks_RayTracerKernel The number of ticks for which kernel "RayTracerKernel" will run.
 * \param [in] instream_rays_in Stream "rays_in".
 * \param [in] instream_size_rays_in The size of the stream instream_rays_in in bytes.
 * \param [in] instream_triangles_in Stream "triangles_in".
 * \param [in] instream_size_triangles_in The size of the stream instream_triangles_in in bytes.
 * \param [out] outstream_results_out Stream "results_out".
 * \param [in] outstream_size_results_out The size of the stream outstream_results_out in bytes.
 */
void RayTracer(
	uint64_t ticks_RayTracerKernel,
	const void *instream_rays_in,
	size_t instream_size_rays_in,
	const void *instream_triangles_in,
	size_t instream_size_triangles_in,
	void *outstream_results_out,
	size_t outstream_size_results_out);

/**
 * \brief Basic static non-blocking function for the interface 'default'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [in] ticks_RayTracerKernel The number of ticks for which kernel "RayTracerKernel" will run.
 * \param [in] instream_rays_in Stream "rays_in".
 * \param [in] instream_size_rays_in The size of the stream instream_rays_in in bytes.
 * \param [in] instream_triangles_in Stream "triangles_in".
 * \param [in] instream_size_triangles_in The size of the stream instream_triangles_in in bytes.
 * \param [out] outstream_results_out Stream "results_out".
 * \param [in] outstream_size_results_out The size of the stream outstream_results_out in bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *RayTracer_nonblock(
	uint64_t ticks_RayTracerKernel,
	const void *instream_rays_in,
	size_t instream_size_rays_in,
	const void *instream_triangles_in,
	size_t instream_size_triangles_in,
	void *outstream_results_out,
	size_t outstream_size_results_out);

/**
 * \brief Advanced static interface, structure for the engine interface 'default'
 * 
 */
typedef struct { 
	uint64_t ticks_RayTracerKernel; /**<  [in] The number of ticks for which kernel "RayTracerKernel" will run. */
	const void *instream_rays_in; /**<  [in] Stream "rays_in". */
	size_t instream_size_rays_in; /**<  [in] The size of the stream instream_rays_in in bytes. */
	const void *instream_triangles_in; /**<  [in] Stream "triangles_in". */
	size_t instream_size_triangles_in; /**<  [in] The size of the stream instream_triangles_in in bytes. */
	void *outstream_results_out; /**<  [out] Stream "results_out". */
	size_t outstream_size_results_out; /**<  [in] The size of the stream outstream_results_out in bytes. */
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

