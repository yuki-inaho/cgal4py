#include "mpi.h"
#include <math.h>
#include <stdint.h>
#include <vector>
#include <set>
#include <map>
#include "c_kdtree.hpp"
#include "c_tools.hpp"
#include "c_delaunay2.hpp"
#include "c_delaunay3.hpp"
#include "c_periodic_delaunay2.hpp"
#include "c_periodic_delaunay3.hpp"
#if (CGAL_VERSION_NR < 1040900900)
#define VALID_PERIODIC_2 0
#else
#define VALID_PERIODIC_2 1
#endif
#if (CGAL_VERSION_NR < 1030501000)
#define VALID_PERIODIC_3 0
#else
#define VALID_PERIODIC_3 1
#endif


template <typename Info_>
class CGeneralDelaunay
{
public:
  typedef Info_ Info;
  typedef Delaunay_with_info_2<Info> Delaunay2;
  typedef Delaunay_with_info_3<Info> Delaunay3;
  typedef PeriodicDelaunay_with_info_2<Info> PeriodicDelaunay2;
  typedef PeriodicDelaunay_with_info_3<Info> PeriodicDelaunay3;
  int ndim;
  bool periodic;
  void *T;

  CGeneralDelaunay(int ndim0, bool periodic0 = false,
		   const double *domain = NULL) {
    ndim = ndim0;
    periodic = periodic0;
    if (ndim == 2) {
      if (periodic)
	T = (void*)(new PeriodicDelaunay2(domain));
      else
	T = (void*)(new Delaunay2());
    } else if (ndim == 3) {
      if (periodic)
	T = (void*)(new PeriodicDelaunay3(domain));
      else
	T = (void*)(new Delaunay3());
    }
  }

  uint32_t num_finite_verts() {
    uint32_t out = 0;
    if (ndim == 2) {
      if (periodic)
	out = ((PeriodicDelaunay2*)T)->num_finite_verts();
      else
	out = ((Delaunay2*)T)->num_finite_verts();
    } else if (ndim == 3) {
      if (periodic)
	out = ((PeriodicDelaunay3*)T)->num_finite_verts();
      else
	out = ((Delaunay3*)T)->num_finite_verts();
    }
    return out;
  }
  uint32_t num_cells() {
    uint32_t out = 0;
    if (ndim == 2) {
      if (periodic)
	out = ((PeriodicDelaunay2*)T)->num_cells();
      else
	out = ((Delaunay2*)T)->num_cells();
    } else if (ndim == 3) {
      if (periodic)
	out = ((PeriodicDelaunay3*)T)->num_cells();
      else
	out = ((Delaunay3*)T)->num_cells();
    }
    return out;
  }

  void insert(double *pts, Info *val, uint32_t n) {
    if (ndim == 2) {
      if (periodic)
	((PeriodicDelaunay2*)T)->insert(pts, val, n);
      else
	((Delaunay2*)T)->insert(pts, val, n);
    } else if (ndim == 3) {
      if (periodic)
	((PeriodicDelaunay3*)T)->insert(pts, val, n);
      else
	((Delaunay3*)T)->insert(pts, val, n);
    }
  }

  template <typename I>
  I serialize_info2idx(I &n, I &m, int32_t &d,
                       I* cells, I* neighbors,
                       Info max_info, I* idx,
		       int32_t* offsets = NULL,
                       double* domain = NULL,
		       int32_t* cover = NULL) const {
    I out = 0;
    if (ndim == 2) {
      if (periodic)
	out = ((PeriodicDelaunay2*)T)->serialize_info2idx(n, m, d, domain, cover,
							  cells, neighbors,
							  offsets, max_info, idx);
      else
	out = ((Delaunay2*)T)->serialize_info2idx(n, m, d, cells, neighbors,
						  max_info, idx);
    } else if (ndim == 3) {
      if (periodic)
	out = ((PeriodicDelaunay3*)T)->serialize_info2idx(n, m, d, domain, cover,
							  cells, neighbors,
							  offsets, max_info, idx);
      else
	out = ((Delaunay3*)T)->serialize_info2idx(n, m, d, cells, neighbors,
						  max_info, idx);
    }
    return out;
  }

  template <typename I>
  void deserialize(I n, I m, int32_t d,
                   double* vert_pos, Info* vert_info,
                   I* cells, I* neighbors, I idx_inf,
		   double* domain = NULL, int32_t* cover = NULL,
		   int32_t* offsets = NULL) {
    if (ndim == 2) {
      if (periodic)
	((PeriodicDelaunay2*)T)->deserialize(n, m, d, domain, cover,
					     vert_pos, vert_info,
					     cells, neighbors,
					     offsets, idx_inf);
      else
	((Delaunay2*)T)->deserialize(n, m, d, vert_pos, vert_info,
				     cells, neighbors, idx_inf);
    } else if (ndim == 3) {
      if (periodic)
	((PeriodicDelaunay3*)T)->deserialize(n, m, d, domain, cover,
					     vert_pos, vert_info,
					     cells, neighbors,
					     offsets, idx_inf);
      else
	((Delaunay3*)T)->deserialize(n, m, d, vert_pos, vert_info,
				     cells, neighbors, idx_inf);
    }
  }

  std::vector<std::vector<Info>> outgoing_points(uint64_t nbox,
                                                 double *left_edges,
						 double *right_edges) const {
    std::vector<std::vector<Info>> out;
    if (ndim == 2) {
      if (periodic)
	out = ((PeriodicDelaunay2*)T)->outgoing_points(nbox, left_edges, right_edges);
      else
	out = ((Delaunay2*)T)->outgoing_points(nbox, left_edges, right_edges);
    } else if (ndim == 3) {
      if (periodic)
	out = ((PeriodicDelaunay3*)T)->outgoing_points(nbox, left_edges, right_edges);
      else
	out = ((Delaunay3*)T)->outgoing_points(nbox, left_edges, right_edges);
    }
    return out;
  }

};


template <typename Info_>
class CParallelLeaf
{
public:
  typedef Info_ Info;
  typedef CGeneralDelaunay<Info> Delaunay;
  // typedef Delaunay_with_info_2<Info> Delaunay;
  bool from_node;
  int size;
  int rank;
  uint32_t id;
  uint32_t nleaves;
  uint32_t ndim;
  uint64_t npts_orig = 0;
  uint64_t npts = 0;
  Info *idx = NULL;
  double *pts = NULL;
  double *le = NULL;
  double *re = NULL;
  int *periodic_le = NULL;
  int *periodic_re = NULL;
  double *domain_width = NULL;
  int nneigh;
  uint32_t *neigh = NULL;
  double *neigh_le;
  double *neigh_re;
  double *leaves_le;
  double *leaves_re;
  Delaunay *T = NULL;
  std::set<uint32_t> *all_neigh;
  std::vector<std::set<uint32_t>> *lneigh;
  std::vector<std::set<uint32_t>> *rneigh;

  CParallelLeaf(uint32_t nleaves0, uint32_t ndim0, int src) {
    uint32_t k;
    MPI_Comm_size ( MPI_COMM_WORLD, &size);
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
    from_node = false;
    nleaves = nleaves0;
    ndim = ndim0;
    neigh = (uint32_t*)malloc(nleaves*sizeof(uint32_t));
    neigh_le = (double*)malloc(nleaves*ndim*sizeof(double*));
    neigh_re = (double*)malloc(nleaves*ndim*sizeof(double*));
    leaves_le = (double*)malloc(nleaves*ndim*sizeof(double*));
    leaves_re = (double*)malloc(nleaves*ndim*sizeof(double*));
    all_neigh = new std::set<uint32_t>();
    lneigh = new std::vector<std::set<uint32_t>>();
    rneigh = new std::vector<std::set<uint32_t>>();
    for (k = 0; k < ndim; k++) {
      lneigh->push_back(std::set<uint32_t>());
      rneigh->push_back(std::set<uint32_t>());
    }
    // Receive leaf info from root process
    recv(src);
    int i;
    uint32_t n;
    for (i = 0; i < nneigh; i++) {
      n = neigh[i];
      memcpy(neigh_le+ndim*i, leaves_le+ndim*n, ndim*sizeof(double));
      memcpy(neigh_re+ndim*i, leaves_re+ndim*n, ndim*sizeof(double));
    }
  };

  CParallelLeaf(uint32_t nleaves0, uint32_t ndim0, KDTree* tree, int index) {
    uint32_t k;
    MPI_Comm_size ( MPI_COMM_WORLD, &size);
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
    from_node = true;
    nleaves = nleaves0;
    ndim = ndim0;
    neigh = (uint32_t*)malloc(nleaves*sizeof(uint32_t));
    neigh_le = (double*)malloc(nleaves*ndim*sizeof(double*));
    neigh_re = (double*)malloc(nleaves*ndim*sizeof(double*));
    leaves_le = (double*)malloc(nleaves*ndim*sizeof(double*));
    leaves_re = (double*)malloc(nleaves*ndim*sizeof(double*));
    all_neigh = new std::set<uint32_t>();
    lneigh = new std::vector<std::set<uint32_t>>();
    rneigh = new std::vector<std::set<uint32_t>>();
    for (k = 0; k < ndim; k++) {
      lneigh->push_back(std::set<uint32_t>());
      rneigh->push_back(std::set<uint32_t>());
    }
    // Transfer leaf information
    Node* node = tree->leaves[index];
    std::vector<uint32_t>::iterator it;
    int i;
    uint64_t j;
    uint32_t n;
    id = node->leafid;
    npts = node->children;
    idx = (Info*)malloc(npts*sizeof(Info));
    pts = (double*)malloc(ndim*npts*sizeof(double));
    le = node->left_edge;
    re = node->right_edge;
    periodic_le = (int*)malloc(ndim*sizeof(int));
    periodic_re = (int*)malloc(ndim*sizeof(int));
    domain_width = tree->domain_width;
    nneigh = (int)(node->all_neighbors.size());
    for (j = 0; j < npts; j++) {
      idx[j] = (Info)(node->left_idx + j);
      for (k = 0; k < ndim; k++) {
    	pts[ndim*j+k] = tree->all_pts[ndim*tree->all_idx[node->left_idx+j]+k];
      }
    }
    for (k = 0; k < nleaves; k++) {
      memcpy(leaves_le+ndim*k, tree->leaves[k]->left_edge,
    	     ndim*sizeof(double));
      memcpy(leaves_re+ndim*k, tree->leaves[k]->right_edge,
    	     ndim*sizeof(double));
    }
    for (i = 0; i < nneigh; i++) {
      neigh[i] = node->all_neighbors[i];
    }
    for (k = 0; k < ndim; k++) {
      periodic_le[k] = node->periodic_left[k];
      periodic_re[k] = node->periodic_right[k];
    }
    for (k = 0; k < ndim; k++) {
      (*lneigh)[k].insert(node->left_neighbors[k].begin(),
    			  node->left_neighbors[k].end());
      (*rneigh)[k].insert(node->right_neighbors[k].begin(),
    			  node->right_neighbors[k].end());
    }
    // Shift edges of periodic neighbors
    for (k = 0; k < ndim; k++) {
      if (periodic_le[k]) {
    	for (std::set<uint32_t>::iterator it = (*lneigh)[k].begin();
    	     it != (*lneigh)[k].end(); it++) {
    	  leaves_le[*it, k] -= domain_width[k];
    	  leaves_re[*it, k] -= domain_width[k];
    	}
      }
      if (periodic_re[k]) {
    	for (std::set<uint32_t>::iterator it = (*rneigh)[k].begin();
    	     it != (*rneigh)[k].end(); it++) {
    	  leaves_le[*it, k] += domain_width[k];
    	  leaves_re[*it, k] += domain_width[k];
    	}
      }
    }
    // Select edges of neighbors
    for (i = 0; i < nneigh; i++) {
      n = neigh[i];
      memcpy(neigh_le+ndim*i, leaves_le+ndim*n, ndim*sizeof(double));
      memcpy(neigh_re+ndim*i, leaves_re+ndim*n, ndim*sizeof(double));
    }
  }

  ~CParallelLeaf() {
    // free(neigh);
    // free(neigh_le);
    // free(neigh_re);
    // free(leaves_le);
    // free(leaves_re);
    // if (from_node) {
    //   free(pts);
    //   free(idx);
    //   free(periodic_le);
    //   free(periodic_re);
    // } else {
    //   free(pts);
    //   free(idx);
    //   free(periodic_le);
    //   free(periodic_re);
    //   free(le);
    //   free(re);
    //   free(domain_width);
    // }
  }

  uint32_t num_cells() {
    return T->num_cells();
  }

  void send(int dst) {
    int i = 0, j;
    uint32_t k = 0;
    MPI_Send(&id, 1, MPI_UNSIGNED, dst, i++, MPI_COMM_WORLD);
    MPI_Send(&npts, 1, MPI_UNSIGNED_LONG, dst, i++, MPI_COMM_WORLD);
    if (sizeof(Info) == sizeof(uint32_t))
      MPI_Send(idx, npts, MPI_UNSIGNED, dst, i++, MPI_COMM_WORLD);
    else
      MPI_Send(idx, npts, MPI_UNSIGNED_LONG, dst, i++, MPI_COMM_WORLD);
    MPI_Send(pts, ndim*npts, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    MPI_Send(le, ndim, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    MPI_Send(re, ndim, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    MPI_Send(periodic_le, ndim, MPI_INT, dst, i++, MPI_COMM_WORLD);
    MPI_Send(periodic_re, ndim, MPI_INT, dst, i++, MPI_COMM_WORLD);
    MPI_Send(domain_width, ndim, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    MPI_Send(&nneigh, 1, MPI_INT, dst, i++, MPI_COMM_WORLD);
    MPI_Send(neigh, nneigh, MPI_UNSIGNED, dst, i++, MPI_COMM_WORLD);
    MPI_Send(leaves_le, nleaves*ndim, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    MPI_Send(leaves_re, nleaves*ndim, MPI_DOUBLE, dst, i++, MPI_COMM_WORLD);
    uint32_t *dummy = (uint32_t*)malloc(nneigh*sizeof(uint32_t));
    int ndum;
    for (k = 0; k < ndim; k++) {
      // left neighbors
      ndum = (int)((*lneigh)[k].size());
      j = 0;
      for (std::set<uint32_t>::iterator it = (*lneigh)[k].begin();
    	   it != (*lneigh)[k].end(); it++) {
    	dummy[j] = *it;
    	j++;
      }
      MPI_Send(&ndum, 1, MPI_INT, dst, i++, MPI_COMM_WORLD);
      MPI_Send(dummy, ndum, MPI_UNSIGNED, dst, i++, MPI_COMM_WORLD);
      // right neighbors
      ndum = (int)((*rneigh)[k].size());
      j = 0;
      for (std::set<uint32_t>::iterator it = (*rneigh)[k].begin();
    	   it != (*rneigh)[k].end(); it++) {
    	dummy[j] = *it;
    	j++;
      }
      MPI_Send(&ndum, 1, MPI_INT, dst, i++, MPI_COMM_WORLD);
      MPI_Send(dummy, ndum, MPI_UNSIGNED, dst, i++, MPI_COMM_WORLD);
    }
    free(dummy);
  };

  void recv(int src) {
    int i = 0, j;
    uint32_t k = 0;
    MPI_Recv(&id, 1, MPI_UNSIGNED, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(&npts, 1, MPI_UNSIGNED_LONG, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    idx = (Info*)malloc(npts*sizeof(Info));
    pts = (double*)malloc(ndim*npts*sizeof(double));
    le = (double*)malloc(ndim*sizeof(double));
    re = (double*)malloc(ndim*sizeof(double));
    periodic_le = (int*)malloc(ndim*sizeof(int));
    periodic_re = (int*)malloc(ndim*sizeof(int));
    domain_width = (double*)malloc(ndim*sizeof(double));
    if (sizeof(Info) == sizeof(uint32_t))
      MPI_Recv(idx, npts, MPI_UNSIGNED, src, i++, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
    else
      MPI_Recv(idx, npts, MPI_UNSIGNED_LONG, src, i++, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
    MPI_Recv(pts, ndim*npts, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(le, ndim, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(re, ndim, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(periodic_le, ndim, MPI_INT, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(periodic_re, ndim, MPI_INT, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(domain_width, ndim, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(&nneigh, 1, MPI_INT, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(neigh, nneigh, MPI_UNSIGNED, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(leaves_le, nleaves*ndim, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    MPI_Recv(leaves_re, nleaves*ndim, MPI_DOUBLE, src, i++, MPI_COMM_WORLD,
    	     MPI_STATUS_IGNORE);
    uint32_t *dummy = (uint32_t*)malloc(nneigh*sizeof(uint32_t));
    int ndum;
    for (k = 0; k < ndim; k++) {
      // left neighbors
      MPI_Recv(&ndum, 1, MPI_INT, src, i++, MPI_COMM_WORLD,
    	       MPI_STATUS_IGNORE);
      MPI_Recv(dummy, ndum, MPI_UNSIGNED, src, i++, MPI_COMM_WORLD,
    	       MPI_STATUS_IGNORE);
      for (j = 0; j < ndum; j++) {
    	(*lneigh)[k].insert(dummy[j]);
      }
      // right neighbors
      MPI_Recv(&ndum, 1, MPI_INT, src, i++, MPI_COMM_WORLD,
    	       MPI_STATUS_IGNORE);
      MPI_Recv(dummy, ndum, MPI_UNSIGNED, src, i++, MPI_COMM_WORLD,
    	       MPI_STATUS_IGNORE);
      for (j = 0; j < ndum; j++) {
    	(*rneigh)[k].insert(dummy[j]);
      }
    }
    free(dummy);
  }

  void init_triangulation() {
    T = new Delaunay(ndim, false);
    // Insert points using monotonic indices
    Info *idx_dum = (Info*)malloc(npts*sizeof(Info));
    for (Info i = 0; i < npts; i++)
      idx_dum[i] = i;
    T->insert(pts, idx_dum, npts);
    free(idx_dum);
    npts_orig = npts;
  }

  void insert(double *pts_new, Info *idx_new, uint64_t npts_new) {
    // Insert points
    Info *idx_dum = (Info*)malloc(npts_new*sizeof(Info));
    for (Info i = 0, j = npts; i < npts_new; i++, j++)
      idx_dum[i] = j;
    T->insert(pts_new, idx_dum, npts_new);
    free(idx_dum);
    // Copy indices
    idx = (Info*)realloc(idx, (npts+npts_new)*sizeof(Info));
    memcpy(idx+npts, idx_new, npts_new*sizeof(Info));
    // Copy points
    pts = (double*)realloc(pts, ndim*(npts+npts_new)*sizeof(double));
    memcpy(pts+ndim*npts, pts_new, ndim*npts_new*sizeof(double));
    // Advance count
    npts += npts_new;
  }
  
  template <typename I>
  I serialize(I &n, I &m,
	      I *cells, I *neigh,
	      uint32_t *idx_verts, uint64_t *idx_cells,
	      bool sort = false) {
    n = (Info)(T->num_finite_verts());
    m = (Info)(T->num_cells());
    int32_t d = ndim;
    I idx_inf = T->serialize_info2idx(n, m, d, cells, neigh,
				      (Info)npts_orig, idx);
    if (sort) {
      sortSerializedTess(cells, neigh, m, d+1);
    } else {
      uint64_t j;
      uint32_t k;
      for (j = 0; j < (uint64_t)m; j++) {
    	idx_cells[j] = j;
    	for (k = 0; k < (uint32_t)(d+1); k++)
    	  idx_verts[(d+1)*j+k] = k;
      }
      arg_sortSerializedTess(cells, m, d+1, idx_verts, idx_cells);
    }
    return idx_inf;
  };

  void outgoing_points(std::vector<std::vector<uint32_t>> &src_out,
		       std::vector<std::vector<uint32_t>> &dst_out,
		       std::vector<std::vector<uint32_t>> &cnt_out,
		       std::vector<std::vector<uint32_t>> &nct_out,
		       std::vector<Info*> &idx_out,
		       std::vector<double*> &pts_out,
		       std::vector<uint32_t*> &ngh_out) {
    int i, j;
    uint32_t k, n, dst, src=id;
    int task;
    typedef typename std::vector<Info> vect_Info;
    std::vector<uint32_t>::iterator it32;
    typename vect_Info::iterator it;
    std::vector<vect_Info> out_leaves;
    out_leaves = T->outgoing_points(nneigh, neigh_le, neigh_re);
    uint32_t ntot = 0;
    uint32_t nold, nnew, nold_neigh, nnew_neigh;
    for (i = 0; i < nneigh; i++) {
      dst = neigh[i];
      task = dst % size;
      src_out[task].push_back(src);
      dst_out[task].push_back(dst);
      for (it = out_leaves[i].begin(); it != out_leaves[i].end(); ) {
	if (*it < npts_orig)
	  it++;
	else
	  it = out_leaves[i].erase(it);
      }
      nnew = (uint32_t)(out_leaves[i].size());
      nold = 0;
      for (it32 = cnt_out[task].begin();
	   it32 != cnt_out[task].end(); it32++)
	nold += *it32;
      if (nnew > 0)
	nnew_neigh = nneigh;
      else
	nnew_neigh = 0;
      nold_neigh = 0;
      for (it32 = nct_out[task].begin();
	   it32 != nct_out[task].end(); it32++)
	nold_neigh += *it32;
      // TODO: Maybe move realloc outside of loop
      idx_out[task] = (Info*)realloc(idx_out[task],
				     (nold+nnew)*sizeof(Info));
      pts_out[task] = (double*)realloc(pts_out[task],
				       ndim*(nold+nnew)*sizeof(double));
      ngh_out[task] = (uint32_t*)realloc(ngh_out[task],
					 (nold_neigh+nnew_neigh)*sizeof(uint32_t));
      cnt_out[task].push_back(nnew);
      nct_out[task].push_back(nnew_neigh);
      for (it = out_leaves[i].begin(), j = nold;
	   it != out_leaves[i].end(); it++, j++) {
	idx_out[task][j] = idx[*it];
	for (k = 0; k < ndim; k++)
	  pts_out[task][ndim*j+k] = pts[ndim*(*it)+k];
      }
      ntot += nnew;
      if (nnew_neigh > 0) {
	memcpy(ngh_out[task]+nold_neigh, neigh, nnew_neigh*sizeof(uint32_t));
      }
    }
    // Transfer neighbors to log & reset count to 0
    for (i = 0; i < nneigh; i++) {
      n = neigh[i];
      all_neigh->insert(n);
    }
    nneigh = 0;
  }

  void incoming_points(uint32_t src, uint32_t nnpts_recv,
		       uint32_t nneigh_recv, Info *idx_recv,
		       double *pts_recv, uint32_t *neigh_recv) {
    if (nnpts_recv == 0)
      return;
    uint64_t j;
    uint32_t k;
    if (src == id) {
      for (k = 0; k < ndim; k++) {
	if (periodic_le[k] and periodic_re[k]) {
	  for (j = 0; j < nnpts_recv; j++) {
	    if ((pts_recv[ndim*j+k] - le[k]) < (re[k] - pts_recv[ndim*j+k])) {
	      pts_recv[ndim*j+k] += domain_width[k];
	    }
	    if ((re[k] - pts_recv[ndim*j+k]) < (pts_recv[ndim*j+k] - le[k])) {
	      pts_recv[ndim*j+k] -= domain_width[k];
	    }
	  }
	}
      }
    } else {
      for (k = 0; k < ndim; k++) {
	if (periodic_re[k] and ((*rneigh)[k].count(src) > 0)) {
	  for (j = 0; j < nnpts_recv; j++) {
	    if ((pts_recv[ndim*j+k] + domain_width[k] - re[k]) <
		(le[k] - pts_recv[ndim*j+k]))
	      pts_recv[ndim*j+k] += domain_width[k];
	  }
	}
	if (periodic_le[k] and ((*lneigh)[k].count(src) > 0)) {
	  for (j = 0; j < nnpts_recv; j++) {
	    if ((le[k] - pts_recv[ndim*j+k] + domain_width[k]) <
		(pts_recv[ndim*j+k] - re[k]))
	      pts_recv[ndim*j+k] -= domain_width[k];
	  }
	}
      }
    }
    // Add points to tessellation, then arrays
    insert(pts_recv, idx_recv, nnpts_recv);
    // Add neighbors
    uint32_t n;
    for (k = 0; k < nneigh_recv; k++) {
      n = neigh_recv[k];
      if ((n != id) and (all_neigh->count(n) == 0)) {
	neigh[nneigh] = n;
	memcpy(neigh_le+ndim*nneigh, leaves_le+ndim*n, ndim*sizeof(double));
	memcpy(neigh_re+ndim*nneigh, leaves_re+ndim*n, ndim*sizeof(double));
	nneigh++;
      }
    }
  }

};


template <typename Info_>
class CParallelDelaunay
{
public:
  typedef Info_ Info;
  int rank;
  int size;
  uint32_t ndim;
  int tree_exists = 0;
  // Things only valid for root
  double *le;
  double *re;
  bool *periodic;
  uint64_t npts_prev = 0;
  uint64_t npts_total;
  int nleaves_total;
  double *pts_total = NULL;
  uint64_t *idx_total = NULL;
  Info *info_total;
  KDTree *tree = NULL;
  // Things for each process
  int nleaves;
  std::vector<CParallelLeaf<Info>*> leaves;
  // CParallelLeaf<Info> *leaves = NULL;
  std::map<int,uint32_t> map_id2idx;

  CParallelDelaunay() {}
  CParallelDelaunay(uint32_t ndim0, double *le0, double *re0,
		    bool *periodic0) {
    MPI_Comm_size ( MPI_COMM_WORLD, &size);
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
    ndim = ndim0;
    le = le0;
    re = re0;
    periodic = periodic0;
    if (rank == 0) {
      for (int task = 1; task < size; task ++)
	MPI_Send(&ndim, 1, MPI_UNSIGNED, task, 0, MPI_COMM_WORLD);
    } else {
      MPI_Recv(&ndim, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
    }
  }

  ~CParallelDelaunay() {
    // int i;
    // if (idx_total != NULL)
    //   free(idx_total);
    // for (i = 0; i < nleaves; i++) {
    //   delete(leaves.pop());
    // }
    // delete(tree);
  }

  void insert(uint64_t npts0, double *pts0) {
    int i;
    uint64_t j;
    uint32_t k;
    if (tree_exists == 0) {
      // Initial domain decomposition
      npts_total = npts0;
      pts_total = pts0;
      domain_decomp();
      for (i = 0; i < nleaves; i++)
	leaves[i]->init_triangulation();
    } else {
      Info *iidx = NULL;
      double *ipts = NULL;
      // Assign points to leaves based on initial domain decomp
      if (rank == 0) {
	// Assign each point to an existing leaf
      	std::vector<std::vector<uint64_t>> dist;
      	for (i = 0; i < nleaves_total; i++) {
      	  dist.push_back(std::vector<uint64_t>());
      	  dist[i].reserve(npts0/nleaves_total+1);
      	}
	int cnt = 0;
      	for (j = 0; j < npts0; j++) {
	  Node *res = tree->search(pts0+ndim*j);
	  if (res != NULL)
	    dist[res->leafid].push_back(j);
	  else
	    cnt++;
	}
	if (cnt > 0) {
	    printf("%d points were not within the bounds of the original domain decomposition\n",
		   cnt);
	}
	// Send new points to leaf
      	int nsend, task;
	int iroot = 0;
      	for (i = 0; i < nleaves_total; i++) {
      	  task = i % size;
      	  nsend = (int)(dist[i].size());
	  iidx = (Info*)realloc(iidx, nsend*sizeof(Info));
	  ipts = (double*)realloc(ipts, ndim*nsend*sizeof(double));
	  for (j = 0; j < (uint64_t)nsend; j++) {
	    iidx[j] = dist[i][j] + npts_prev;
	    for (k = 0; k < ndim; k++) 
	      ipts[ndim*j+k] = pts0[ndim*dist[i][j]+k];
	  }
      	  if (task == rank) {
	    leaves[iroot]->insert(ipts, iidx, nsend);
	    iroot++;
      	  } else {
      	    MPI_Send(&nsend, 1, MPI_INT, task, 20+task, MPI_COMM_WORLD);
	    if (sizeof(Info) == sizeof(uint32_t))
	      MPI_Send(iidx, nsend, MPI_UNSIGNED, task, 21+task,
		       MPI_COMM_WORLD);
	    else
	      MPI_Send(iidx, nsend, MPI_UNSIGNED_LONG, task, 21+task,
		       MPI_COMM_WORLD);
	    MPI_Send(ipts, ndim*nsend, MPI_DOUBLE, task, 22+task,
		     MPI_COMM_WORLD);
      	  }
	  free(iidx);
	  free(ipts);
      	}
      } else {
      	int nrecv;
      	for (i = 0; i < nleaves; i++) {
      	  MPI_Recv(&nrecv, 1, MPI_INT, 0, 20+rank, MPI_COMM_WORLD,
      		   MPI_STATUS_IGNORE);
	  iidx = (Info*)realloc(iidx,nrecv*sizeof(Info));
	  ipts = (double*)realloc(ipts,ndim*nrecv*sizeof(double));
	  if (sizeof(Info) == sizeof(uint32_t))
	    MPI_Recv(iidx, nrecv, MPI_UNSIGNED, 0, 21+rank,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  else
	    MPI_Recv(iidx, nrecv, MPI_UNSIGNED_LONG, 0, 21+rank,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  MPI_Recv(ipts, ndim*nrecv, MPI_DOUBLE, 0, 22+rank,
		   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  leaves[i]->insert(ipts, iidx, nrecv);
      	}
      }
      if (iidx != NULL)
	free(iidx);
      if (ipts != NULL)
	free(ipts);
    }
    // Exchange points
    // exchange();
    npts_prev += npts0;
  }

  void exchange() {
    uint64_t nrecv_total = 1;
    uint64_t nrecv;
    int nexch;
    uint32_t *src_recv = NULL;
    uint32_t *dst_recv = NULL;
    uint32_t *cnt_recv = NULL;
    uint32_t *nct_recv = NULL;
    Info *idx_recv = NULL;
    double *pts_recv = NULL;
    uint32_t *ngh_recv = NULL;
    while (nrecv_total != 0) {
      nexch = outgoing_points(&src_recv, &dst_recv, &cnt_recv, &nct_recv,
			      &idx_recv, &pts_recv, &ngh_recv);
      nrecv = incoming_points(nexch, src_recv, dst_recv, cnt_recv, nct_recv,
      			      idx_recv, pts_recv, ngh_recv);
      free(src_recv);
      free(dst_recv);
      free(cnt_recv);
      free(nct_recv);
      // free(idx_recv);  // Memory moved to leaf
      // free(pts_recv);  // Memory moved to leaf
      free(ngh_recv);
      MPI_Allreduce(&nrecv, &nrecv_total, 1, MPI_UNSIGNED, MPI_SUM,
		    MPI_COMM_WORLD);
    }
  }

  uint64_t incoming_points(int nexch, uint32_t *src_recv, uint32_t *dst_recv,
			   uint32_t *cnt_recv, uint32_t *nct_recv,
			   Info *idx_recv, double *pts_recv,
			   uint32_t *ngh_recv) {
    uint64_t nrecv = 0;
    uint64_t nprev_pts = 0, nprev_ngh = 0;
    Info *iidx;
    double *ipts;
    uint32_t *ingh;
    int dst;
    for (int i = 0; i < nexch; i++) {
      iidx = idx_recv + nprev_pts;
      ipts = pts_recv + ndim*nprev_pts;
      ingh = ngh_recv + nprev_ngh;
      dst = map_id2idx[dst_recv[i]];
      leaves[dst]->incoming_points(src_recv[i], cnt_recv[i], nct_recv[i],
				   iidx, ipts, ingh);
      nprev_pts += cnt_recv[i];
      nprev_ngh += nct_recv[i];
    }
    nrecv = nprev_pts;
    return nrecv;
  }

  int outgoing_points(uint32_t **src_recv, uint32_t **dst_recv,
		      uint32_t **cnt_recv, uint32_t **nct_recv,
		      Info **idx_recv, double **pts_recv,
		      uint32_t **ngh_recv) {
    int i, j;
    // Get output from each leaf
    std::vector<std::vector<uint32_t>> src_out, dst_out, cnt_out, nct_out;
    std::vector<Info*> idx_out;
    std::vector<double*> pts_out;
    std::vector<uint32_t*> ngh_out;
    for (i = 0; i < size; i++) {
      src_out.push_back(std::vector<uint32_t>());
      dst_out.push_back(std::vector<uint32_t>());
      cnt_out.push_back(std::vector<uint32_t>());
      nct_out.push_back(std::vector<uint32_t>());
      idx_out.push_back(NULL);
      pts_out.push_back(NULL);
      ngh_out.push_back(NULL);
    }
    for (i = 0; i < nleaves; i++)
      leaves[i]->outgoing_points(src_out, dst_out, cnt_out, nct_out,
				 idx_out, pts_out, ngh_out);
    // Send expected counts
    int *count_send = (int*)malloc(size*sizeof(int));
    int *count_recv = (int*)malloc(size*sizeof(int));
    int *offset_send = (int*)malloc(size*sizeof(int));
    int *offset_recv = (int*)malloc(size*sizeof(int));
    for (i = 0; i < size; i++)
      count_send[i] = (int)(src_out[i].size());
    MPI_Alltoall(count_send, 1, MPI_INT,
    		 count_recv, 1, MPI_INT,
    		 MPI_COMM_WORLD);
    int count_send_tot = 0, count_recv_tot = 0;
    for (i = 0; i < size; i++) {
      count_send_tot += count_send[i];
      count_recv_tot += count_recv[i];
    }
    // Send extra info about each exchange
    uint32_t *src_send = (uint32_t*)malloc(count_send_tot*sizeof(uint32_t));
    uint32_t *dst_send = (uint32_t*)malloc(count_send_tot*sizeof(uint32_t));
    uint32_t *cnt_send = (uint32_t*)malloc(count_send_tot*sizeof(uint32_t));
    uint32_t *nct_send = (uint32_t*)malloc(count_send_tot*sizeof(uint32_t));
    (*src_recv) = (uint32_t*)malloc(count_recv_tot*sizeof(uint32_t));
    (*dst_recv) = (uint32_t*)malloc(count_recv_tot*sizeof(uint32_t));
    (*cnt_recv) = (uint32_t*)malloc(count_recv_tot*sizeof(uint32_t));
    (*nct_recv) = (uint32_t*)malloc(count_recv_tot*sizeof(uint32_t));
    int prev_send = 0, prev_recv = 0;
    for (i = 0; i < size; i++) {
      offset_send[i] = prev_send;
      offset_recv[i] = prev_recv;
      for (j = 0; j < count_send[i]; j++) {
    	src_send[prev_send] = src_out[i][j];
    	dst_send[prev_send] = dst_out[i][j];
    	cnt_send[prev_send] = cnt_out[i][j];
    	nct_send[prev_send] = nct_out[i][j];
    	prev_send++;
      }
      prev_recv += count_recv[i];
    }
    MPI_Alltoallv(src_send, count_send, offset_send, MPI_UNSIGNED,
    		  *src_recv, count_recv, offset_recv, MPI_UNSIGNED,
    		  MPI_COMM_WORLD);
    MPI_Alltoallv(dst_send, count_send, offset_send, MPI_UNSIGNED,
    		  *dst_recv, count_recv, offset_recv, MPI_UNSIGNED,
    		  MPI_COMM_WORLD);
    MPI_Alltoallv(cnt_send, count_send, offset_send, MPI_UNSIGNED,
    		  *cnt_recv, count_recv, offset_recv, MPI_UNSIGNED,
    		  MPI_COMM_WORLD);
    MPI_Alltoallv(nct_send, count_send, offset_send, MPI_UNSIGNED,
    		  *nct_recv, count_recv, offset_recv, MPI_UNSIGNED,
    		  MPI_COMM_WORLD);
    free(src_send);
    free(dst_send);
    free(offset_send);
    free(offset_recv);
    // Get counts/offsets for sending arrays
    int *count_idx_send = (int*)malloc(size*sizeof(int));
    int *count_idx_recv = (int*)malloc(size*sizeof(int));
    int *count_pts_send = (int*)malloc(size*sizeof(int));
    int *count_pts_recv = (int*)malloc(size*sizeof(int));
    int *count_ngh_send = (int*)malloc(size*sizeof(int));
    int *count_ngh_recv = (int*)malloc(size*sizeof(int));
    int *offset_idx_send = (int*)malloc(size*sizeof(int));
    int *offset_idx_recv = (int*)malloc(size*sizeof(int));
    int *offset_pts_send = (int*)malloc(size*sizeof(int));
    int *offset_pts_recv = (int*)malloc(size*sizeof(int));
    int *offset_ngh_send = (int*)malloc(size*sizeof(int));
    int *offset_ngh_recv = (int*)malloc(size*sizeof(int));
    prev_send = 0;
    prev_recv = 0;
    int prev_array_send = 0, prev_array_recv = 0;
    int prev_neigh_send = 0, prev_neigh_recv = 0;
    for (i = 0; i < size; i++) {
      count_idx_send[i] = 0;
      count_idx_recv[i] = 0;
      count_ngh_send[i] = 0;
      count_ngh_recv[i] = 0;
      offset_idx_send[i] = prev_array_send;
      offset_idx_recv[i] = prev_array_recv;
      offset_pts_send[i] = ndim*prev_array_send;
      offset_pts_recv[i] = ndim*prev_array_recv;
      offset_ngh_send[i] = prev_neigh_send;
      offset_ngh_recv[i] = prev_neigh_recv;
      for (j = 0; j < count_send[i]; j++) {
    	count_idx_send[i] += cnt_send[prev_send];
    	count_ngh_send[i] += nct_send[prev_send];
    	prev_send++;
      }
      count_pts_send[i] = ndim*count_idx_send[i];
      prev_array_send += count_idx_send[i];
      prev_neigh_send += count_ngh_send[i];
      for (j = 0; j < count_recv[i]; j++) {
    	count_idx_recv[i] += (*cnt_recv)[prev_recv];
    	count_ngh_recv[i] += (*nct_recv)[prev_recv];
    	prev_recv++;
      }
      count_pts_recv[i] = ndim*count_idx_recv[i];
      prev_array_recv += count_idx_recv[i];
      prev_neigh_recv += count_ngh_recv[i];
    }
    free(count_send);
    free(count_recv);
    free(cnt_send);
    free(nct_send);
    // Allocate for arrays
    (*idx_recv) = (Info*)malloc(prev_array_recv*sizeof(Info));
    (*pts_recv) = (double*)malloc(ndim*prev_array_recv*sizeof(double));
    (*ngh_recv) = (uint32_t*)malloc(prev_neigh_recv*sizeof(uint32_t));
    Info *idx_send = (Info*)malloc(prev_array_send*sizeof(Info));
    double *pts_send = (double*)malloc(ndim*prev_array_send*sizeof(double));
    uint32_t *ngh_send = (uint32_t*)malloc(prev_neigh_send*sizeof(uint32_t));
    Info *idx_send_curr = idx_send;
    double *pts_send_curr = pts_send;
    uint32_t *ngh_send_curr = ngh_send;
    for (i = 0; i < size; i++) {
      if (count_idx_send[i] > 0) {
    	memmove(idx_send_curr, idx_out[i], count_idx_send[i]*sizeof(Info));
    	memmove(pts_send_curr, pts_out[i], count_pts_send[i]*sizeof(double));
    	idx_send_curr += count_idx_send[i];
    	pts_send_curr += count_pts_send[i];
      }
      if (count_ngh_send[i] > 0) {
    	memmove(ngh_send_curr, ngh_out[i], count_ngh_send[i]*sizeof(uint32_t));
    	ngh_send_curr += count_ngh_send[i];
      }
      idx_out[i] = NULL;
      pts_out[i] = NULL;
      ngh_out[i] = NULL;
    }
    // Send arrays
    if (sizeof(Info) == sizeof(uint32_t))
      MPI_Alltoallv(idx_send, count_idx_send, offset_idx_send, MPI_UNSIGNED,
		    *idx_recv, count_idx_recv, offset_idx_recv, MPI_UNSIGNED,
		    MPI_COMM_WORLD);
    else
      MPI_Alltoallv(idx_send, count_idx_send, offset_idx_send, MPI_UNSIGNED_LONG,
		    *idx_recv, count_idx_recv, offset_idx_recv, MPI_UNSIGNED_LONG,
		    MPI_COMM_WORLD);
    MPI_Alltoallv(pts_send, count_pts_send, offset_pts_send, MPI_DOUBLE,
    		  *pts_recv, count_pts_recv, offset_pts_recv, MPI_DOUBLE,
    		  MPI_COMM_WORLD);
    MPI_Alltoallv(ngh_send, count_ngh_send, offset_ngh_send, MPI_UNSIGNED,
    		  *ngh_recv, count_ngh_recv, offset_ngh_recv, MPI_UNSIGNED,
    		  MPI_COMM_WORLD);
    free(count_idx_send);
    free(count_pts_send);
    free(count_ngh_send);
    free(count_idx_recv);
    free(count_pts_recv);
    free(count_ngh_recv);
    free(offset_idx_send);
    free(offset_pts_send);
    free(offset_ngh_send);
    free(offset_idx_recv);
    free(offset_pts_recv);
    free(offset_ngh_recv);
    free(idx_send);
    free(pts_send);
    free(ngh_send);
    return count_recv_tot;
  }

  void domain_decomp() {
    int i;
    uint64_t j;
    uint32_t k;
    int *nleaves_per_proc = NULL;
    if (rank == 0) {
      // Create KDtree
      uint32_t leafsize;
      nleaves_total = size;
      nleaves_total = (int)(pow(2,ceil(log2((float)(nleaves_total)))));
      leafsize = (uint32_t)(npts_total/nleaves_total + 1);
      idx_total = (uint64_t*)malloc(npts_total*sizeof(uint64_t));
      for (j = 0; j < npts_total; j++)
	idx_total[j] = j;
      tree = new KDTree(pts_total, idx_total, npts_total, ndim,
		        leafsize, le, re, periodic, false);
      info_total = (Info*)malloc(npts_total*sizeof(Info));
      for (j = 0; j < npts_total; j++)
	info_total[j] = idx_total[j];
      nleaves_total = tree->num_leaves;
    }
    MPI_Bcast(&nleaves_total, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Send number of leaves
    if (rank == 0) {
      nleaves_per_proc = (int*)malloc(sizeof(int)*size);
      for (i = 0; i < size; i++)
	nleaves_per_proc[i] = 0;
      for (k = 0; k < tree->num_leaves; k++) {
	nleaves_per_proc[k % size]++;
      }
    }
    MPI_Scatter(nleaves_per_proc, 1, MPI_INT,
		&nleaves, 1, MPI_INT,
		0, MPI_COMM_WORLD);
    // leaves = (CParallelLeaf<Info>*)malloc(nleaves*sizeof(CParallelLeaf<Info>));
    // Send leaves
    if (rank == 0) {
      int task;
      int iroot = 0;
      for (i = 0; i < nleaves_total; i++) {
	task = i % size;
	if (task == rank) {
	  leaves.push_back(new CParallelLeaf<Info>(nleaves_total, ndim, tree, i));
	  map_id2idx[leaves[iroot]->id] = iroot;
	  iroot++;
	} else {
	  CParallelLeaf<Info> ileaf(nleaves_total, ndim, tree, i);
	  ileaf.send(task);
	}
      }
      tree_exists = 1;
      for (task = 1; task < size; task++)
	MPI_Send(&tree_exists, 1, MPI_INT, task, 33, MPI_COMM_WORLD);
    } else {
      for (i = 0; i < nleaves; i++) {
	leaves.push_back(new CParallelLeaf<Info>(nleaves_total, ndim, 0)); // calls recv
	map_id2idx[leaves[i]->id] = i;
      }
      MPI_Recv(&tree_exists, 1, MPI_INT, 0, 33, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
    }
    if (nleaves_per_proc != NULL)
      free(nleaves_per_proc);
  }

  uint32_t num_cells() {
    int i;
    uint32_t tot_ncells = 0;
    uint32_t tot_ncells_total = 0;
    for (i = 0; i < nleaves; i++)
      tot_ncells += leaves[i]->num_cells();
    MPI_Reduce(&tot_ncells, &tot_ncells_total, 1, MPI_UNSIGNED,
	       MPI_SUM, 0, MPI_COMM_WORLD);
    return tot_ncells_total;
  }

  uint64_t consolidate_tess(uint64_t tot_ncells_total, Info *tot_idx_inf,
			    Info *allverts, Info *allneigh) {
    int i, task, s;
    uint64_t j;
    Info tn = 0, tm = 0;
    Info *verts = NULL, *neigh = NULL;
    uint32_t *idx_verts = NULL;
    uint64_t *idx_cells = NULL;
    Info idx_inf = 0;
    Info *header = (Info*)malloc(3*sizeof(Info));
    // Get counts
    uint64_t max_ncells = 0;
    uint64_t max_ncells_total;
    uint64_t incells;
    uint64_t out = 0;
    for (i = 0; i < nleaves; i++) {
      incells = leaves[i]->num_cells();
      if (incells > max_ncells)
	max_ncells = incells;
    }
    MPI_Allreduce(&max_ncells, &max_ncells_total, 1, MPI_UNSIGNED_LONG,
		  MPI_MAX, MPI_COMM_WORLD);
    // Preallocate
    verts = (Info*)malloc(max_ncells_total*(ndim+1)*sizeof(Info));
    neigh = (Info*)malloc(max_ncells_total*(ndim+1)*sizeof(Info));
    idx_verts = (uint32_t*)malloc(max_ncells_total*(ndim+1)*sizeof(uint32_t));
    idx_cells = (uint64_t*)malloc(max_ncells_total*sizeof(uint64_t));
    // Send serialized info
    if (rank == 0) {
      // Prepare object to hold consolidated tess info
      idx_inf = std::numeric_limits<Info>::max();
      for (j = 0; j < tot_ncells_total*(ndim+1); j++) {
    	allverts[j] = idx_inf;
    	allneigh[j] = idx_inf;
      }
      ConsolidatedLeaves<Info> cons;
      SerializedLeaf<Info> sleaf;
      uint64_t idx_start, idx_stop;
      cons = ConsolidatedLeaves<Info>(ndim, idx_inf,
				      (int64_t)tot_ncells_total,
				      allverts, allneigh);
      // Receive other leaves
      for (i = 0; i < nleaves_total; i++) {
    	task = i % size;
    	if (task == rank) {
    	  idx_inf = leaves[i/size]->serialize(tn, tm, verts, neigh,
					      idx_verts, idx_cells);
    	} else {
    	  s = 0;
	  if (sizeof(Info) == sizeof(uint32_t))
	    MPI_Recv(header, 3, MPI_UNSIGNED, task, s++, MPI_COMM_WORLD,
		     MPI_STATUS_IGNORE);
	  else
	    MPI_Recv(header, 3, MPI_UNSIGNED_LONG, task, s++, MPI_COMM_WORLD,
		     MPI_STATUS_IGNORE);
    	  tn = header[0], tm = header[1], idx_inf = header[2];
	  if (sizeof(Info) == sizeof(uint32_t)) {
	    MPI_Recv(verts, tm*(ndim+1), MPI_UNSIGNED, task, s++,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    MPI_Recv(neigh, tm*(ndim+1), MPI_UNSIGNED, task, s++,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  } else {
	    MPI_Recv(verts, tm*(ndim+1), MPI_UNSIGNED_LONG, task, s++,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    MPI_Recv(neigh, tm*(ndim+1), MPI_UNSIGNED_LONG, task, s++,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  }
    	  MPI_Recv(idx_verts, tm*(ndim+1), MPI_UNSIGNED, task, s++,
    		   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    	  MPI_Recv(idx_cells, tm, MPI_UNSIGNED_LONG, task, s++,
    		   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    	}
    	// Insert serialized leaf
    	idx_start = tree->leaves[i]->left_idx;
    	idx_stop = idx_start + tree->leaves[i]->children;
    	sleaf = SerializedLeaf<Info>(i, ndim, (int64_t)tm, idx_inf,
				     verts, neigh,
				     idx_verts, idx_cells,
				     idx_start, idx_stop);
    	cons.add_leaf(sleaf);
      }
      // Finalize consolidated object
      cons.add_inf();
      out = (uint64_t)cons.ncells;
      (*tot_idx_inf) = cons.idx_inf;
      // CGeneralDelaunay<Info> *Tout = new CGeneralDelaunay<Info>(ndim, false);
      // Tout->deserialize(npts_total, (uint64_t)cons.ncells, (int64_t)ndim,
      // 			pts_total, info_total, allverts, allneigh,
      // 			cons.idx_inf);
      // out = Tout->T;
      // free(allverts);
      // free(allneigh);
    } else {
      (*tot_idx_inf) = 0;
      // Send leaves to root
      for (i = 0; i < nleaves; i++) {
    	idx_inf = leaves[i]->serialize(tn, tm, verts, neigh,
    				       idx_verts, idx_cells);
    	header[0] = tn, header[1] = tm, header[2] = idx_inf;
    	s = 0;
	if (sizeof(Info) == sizeof(uint32_t)) {
	  MPI_Send(header, 3, MPI_UNSIGNED, 0, s++, MPI_COMM_WORLD);
	  MPI_Send(verts, tm*(ndim+1), MPI_UNSIGNED, 0, s++, MPI_COMM_WORLD);
	  MPI_Send(neigh, tm*(ndim+1), MPI_UNSIGNED, 0, s++, MPI_COMM_WORLD);
	} else {
	  MPI_Send(header, 3, MPI_UNSIGNED_LONG, 0, s++, MPI_COMM_WORLD);
	  MPI_Send(verts, tm*(ndim+1), MPI_UNSIGNED_LONG, 0, s++, MPI_COMM_WORLD);
	  MPI_Send(neigh, tm*(ndim+1), MPI_UNSIGNED_LONG, 0, s++, MPI_COMM_WORLD);
	}
    	MPI_Send(idx_verts, tm*(ndim+1), MPI_UNSIGNED, 0, s++, MPI_COMM_WORLD);
    	MPI_Send(idx_cells, tm, MPI_UNSIGNED_LONG, 0, s++, MPI_COMM_WORLD);
      }

    }
    free(header);
    free(verts);
    free(neigh);
    free(idx_verts);
    free(idx_cells);
    return out;
  }

};
