from turbogears import config, expose
import feedparser
import model
import user as srusers

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
	
	user = srusers.User()
	def __init__(self):
		self.feedurl = "default.rss"

	@expose("json")
	@srusers.require(srusers.in_team())
	def getmessages(self):
		return sfd.GetMessages()

	@expose("json")
	@srusers.require(srusers.in_team())
	def milestones(self):
		#TOOD: get this dynamically, perhaps from an xml file? 
		return dict(start=sr_timeline_start, end=sr_timeline_end, events=sr_timeline_events)

	@expose("json")
	@srusers.require(srusers.in_team())
	def setblogfeed(self, feedurl):
	"""Returns the new rss feed url associated with a user. If
	the user has not previously registered a url, it will assign a
	new row in the table, else it will update their existing entry.	
	"""	
		#get the current user
		cur_user = srusers.get_curuser()

		# grab the sql record, edit the url, commit it 
		try:
			r = model.UserBlogFeeds.selectBy(user=cur_user)
			try:
				row = r.getOne()
				if feedurl != row.url:
					row.valid = False	# will need to be re-validated
					row.url = feedurl
					row.set()
				else:
					pass	#nothing to update
			except:
				# user doen't have an entry yet, so create one
				if r.count() == 0:
					new_row = model.UserBlogFeeds(user=cur_user, url=feedurl, valid=False)
					new_row.set()
					return dict(feedurl=feedurl, error=0)
		except:
			return dict(feedurl="", error=1)
		else:
			return dict(feedurl=row.url, error=0)	

	@expose("json")
	@srusers.require(srusers.in_team())
	def getblogfeed(self):
	"""Returns the current rss feed url associated with a user. If 
	the user has not previously registered a url, it will return a
	blank string.
	"""	
		#get the current user
		cur_user = srusers.get_curuser()

		# grab the sql record for the user, if it exists and extract the url
		try:
			r = model.UserBlogFeeds.selectBy(user=cur_user)
			try:
				result = r.getOne().url
			except:
				#did not get a single result, error!
				return dict(feedurl="", error=1)
		except:
			# the record doesn't exist, return blank 
			return dict(feedurl="", error=1)

		return dict(feedurl=result, error=0)
	
	@expose("json")
	@srusers.require(srusers.in_team())
	def getblogposts(self):
		return sbp.GetBlogPosts()
