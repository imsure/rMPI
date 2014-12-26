runs = read.csv( 'rmpi-rb.csv', header=TRUE )

p1 = ggplot( data=runs, 
             aes(x=run, y=all.alive, group=protocol, color=protocol)) 
p1 = p1 + geom_line() + geom_point() + ylim(0,10) +
  xlab('Runs') + ylab('Exec Time (s)') + ggtitle('All nodes alive') +
  scale_x_continuous(breaks=seq(0,10,1))

p2 = ggplot( data=runs, 
             aes(x=run, y=kill.2.primary, group=protocol, color=protocol)) 
p2 = p2 + geom_line() + geom_point() + ylim(0,10) +
  xlab('Runs') + ylab('Exec Time (s)') + 
  ggtitle('1 primary node killed') +
  scale_x_continuous(breaks=seq(0,10,1))

p3 = ggplot( data=runs, 
             aes(x=run, y=kill.2.replica, group=protocol, color=protocol)) 
p3 = p3 + geom_line() + geom_point() + ylim(0,10) +
  xlab('Runs') + ylab('Exec Time (s)') + 
  ggtitle('1 replica node killed') +
  scale_x_continuous(breaks=seq(0,10,1))

p4 = ggplot( data=runs, 
             aes(x=run, y=kill.both, group=protocol, color=protocol)) 
p4 = p4 + geom_line() + geom_point() + ylim(0,10) +
  xlab('Runs') + ylab('Exec Time (s)') + 
  ggtitle('1 primary and 1 replica nodes killed') +
  scale_x_continuous(breaks=seq(0,10,1))

png('rbtest.png', width=1000,height=800)
multiplot(p1, p2, p3, p4, cols=2)
dev.off()

runs.mytest = read.csv( 'rmpi-mytest.csv', header=TRUE )
p5 = ggplot( data=runs.mytest, 
             aes(x=run, y=time, group=protocol, color=protocol)) 
p5 = p5 + geom_line() + geom_point() + ylim(0,20) +
  xlab('Runs') + ylab('Exec Time (s)') + 
  ggtitle('Result of my test') +
  scale_x_continuous(breaks=seq(0,10,1))

png('mytest.png', width=800,height=600)
p5
dev.off()

# Multiple plot function
#
# ggplot objects can be passed in ..., or to plotlist (as a list of ggplot objects)
# - cols:   Number of columns in layout
# - layout: A matrix specifying the layout. If present, 'cols' is ignored.
#
# If the layout is something like matrix(c(1,2,3,3), nrow=2, byrow=TRUE),
# then plot 1 will go in the upper left, 2 will go in the upper right, and
# 3 will go all the way across the bottom.
#
multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
  require(grid)
  
  # Make a list from the ... arguments and plotlist
  plots <- c(list(...), plotlist)
  
  numPlots = length(plots)
  
  # If layout is NULL, then use 'cols' to determine layout
  if (is.null(layout)) {
    # Make the panel
    # ncol: Number of columns of plots
    # nrow: Number of rows needed, calculated from # of cols
    layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
                     ncol = cols, nrow = ceiling(numPlots/cols))
  }
  
  if (numPlots==1) {
    print(plots[[1]])
    
  } else {
    # Set up the page
    grid.newpage()
    pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))
    
    # Make each plot, in the correct location
    for (i in 1:numPlots) {
      # Get the i,j matrix positions of the regions that contain this subplot
      matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))
      
      print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
                                      layout.pos.col = matchidx$col))
    }
  }
}