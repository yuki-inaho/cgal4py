
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection

def plot2D(T, plotfile=None, vertex_kw={}, edge_kw={},
           axs=None, subplot_kw={}, gridspec_kw={}, fig_kw={}, save_kw={},
           title=None, xlabel='x', ylabel='y'):
    r"""Plot a 2D triangulation

    Args:
        T (:class:`Delauany2.Delaunay2`): 2D triangulation class.
        plotfile (:obj:`str`, optional): Full path to file where the plot 
            should be saved. If None, the plot is displayed. Defaults to None.

        vertex_kw (:obj:`dict`, optional): Keywords passed directly to
            :func:`matplotlib.pyplot.scatter`. Defaults to empty dict.
        edge_kw (:obj:`dict`, optional): Keywords passed directly to 
            :class:`matplotlib.collections.LineCollection`. Defaults to empty 
            dict.

        axs (:obj:`matplotlib.pyplot.Axes`, optional): Axes that should be used 
            for plotting. Defaults to None and new axes are created.
        subplot_kw (:obj:`dict`, optional): Keywords passed directly to 
            :meth:`matplotlib.figure.Figure.add_subplot`. Defaults to empty dict.
        gridspec_kw (:obj:`dict`, optional): Keywords passed directly to 
            :class:`matplotlib.gridspec.GridSpec`. Defaults to empty dict.
        fig_kw (:obj:`dict`, optional): Keywords passed directly to
            :func:`matplotlib.pyplot.figure`. Defaults to empty dict.
        save_kw (:obj:`dict`, optional): Keywords passed directly to
            :func:`matplotlib.pyplot.savefig`. Defaults to empty dict.

        title (:obj:`str`, optional): Title that the plot should be given. 
            Defaults to None and no title is displayed.
        xlabel (:obj:`str`, optional): Label for the x-axis. Defaults to 'x'.
        ylabel (:obj:`str`, optional): Label for the y-axis. Defaults to 'y'.

    """
    # Axes creation
    if axs is None:
        plt.close('all')
        fig, axs = plt.subplots(subplot_kw=subplot_kw, gridspec_kw=gridspec_kw,
                                **fig_kw)

    # Labels
    if title is not None:
        axs.set_title(title)
    axs.set_xlabel(xlabel)
    axs.set_ylabel(ylabel)

    # Plot vertices
    v = T.vertices
    axs.scatter(v[:,0], v[:,1], **vertex_kw)

    # Plot edges
    e = T.edges
    seg = [v[e[i,:],:] for i in xrange(e.shape[0])]
    lc = LineCollection(seg, **edge_kw)
    axs.add_collection(lc)

    axs.autoscale()
    axs.margins(0.1)

    # Save
    if plotfile is not None:
        plt.savefig(plotfile, **save_kw)
    else:
        plt.show()
    
    # Return axes
    return axs
