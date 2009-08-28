from turbogears import config, expose
import feedparser

sr_message_feed = "http://sr2010messages.blogspot.com/feeds/posts/default?alt=rss"
sr_timeline_events = [{"date":"September 19, 2009", "title":"Kickstart", "desc":"Kickstart: Competition launch"}, 
			{"date":"November 11, 2009", "title":"Build Chasis", "desc":"You should have the chasis of the robot built"},
			{"date":"November 20, 2009", "title":"Build Ramp", "desc":"You should have built a practice ramp by now"},
			{"date":"February 20, 2010", "title":"Tech Day", "desc":"A chance to get help with the programming/electronics"},
			{"date":"April 20, 2010", "title":"Competition", "desc":"The culmination of all your hard work"}]
sr_timeline_start = "September 19, 2009"
sr_timeline_end = "April 20, 2010"

#We want to sanitize <div>s from feeds
feedparser._HTMLSanitizer.acceptable_elements.remove("a")

# The maximum number of messages to display 
MAX_MESSAGES = 5
# The maixum length of a message to display (in chars)
MAX_MESSAGE_LENGTH = 100

class SRMessageFeed():
	def __init__(self):
		self.fd = feedparser.parse(sr_message_feed)	

	def GetMessages(self):
		msglist = [{"title":e.title, "link":e.link, "author":e.author_detail.name,
					"date":e.date, "body":e.description[0:MAX_MESSAGE_LENGTH].strip()}
				for e in self.fd.entries]
		return dict(msgs=msglist[0:MAX_MESSAGES])

# Single instance of the message feed shared by all users
sfd =  SRMessageFeed()	

class StudentBlogPosts():
	def __init__(self):
		self.feeds = ["http://xgoat.com/wp/feed/", "http://hackaday.com/feed/", "http://groups.google.com/group/srobo/feed/rss_v2_0_msgs.xml"]
		self.msgs = []
		
		# Grab the latest blog post from each feed, display a truncated version of it
		for f in self.feeds:
			fd = feedparser.parse(f)
			try:
				e = fd.entries[0]
				self.msgs.append({"title":e.title, "link":e.link, "author":"TODO:",
					"date":e.date, "body":e.description[0:MAX_MESSAGE_LENGTH].strip()})
			except IndexError:
				pass
		

	def GetBlogPosts(self):
		print self.msgs
		return dict(msgs=self.msgs)

# Single instance of the message feed shared by all users
sfd =  SRMessageFeed()	
sbp = StudentBlogPosts()

class Switchboard(object):
	
	def __init__(self):
		self.feedurl = "default.rss"

	@expose("json")
	def getmessages(self):
		return sfd.GetMessages()

	@expose("json")
	def milestones(self):
		#TOOD: get this dynamically, perhaps from an xml file? 
		return dict(start=sr_timeline_start, end=sr_timeline_end, events=sr_timeline_events)

	@expose("json")
	def setblogfeed(self, feedurl):
		self.feedurl = feedurl
		return dict(feedurl=self.feedurl)
		
	@expose("json")
	def getblogfeed(self):
		return dict(feedurl=self.feedurl)
	
	@expose("json")
	def getblogposts(self):
		return sbp.GetBlogPosts()