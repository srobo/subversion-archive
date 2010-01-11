from turbogears import config, expose
import feedparser
import model
import user as srusers
import time

sr_message_feed = "http://sr2010messages.blogspot.com/feeds/posts/default?alt=rss"
sr_timeline_events = [{"date":"September 19, 2009", "title":"Kickstart", "desc":"Kickstart: Competition launch"},
			{"date":"December 25, 2009", "title":"Christmas", "desc":"Merry Christmas"},
			{"date":"February 20, 2010", "title":"Tech Day", "desc":"A chance to get help with the programming/electronics (provisional date)"},
			{"date":"April 17, 2010", "title":"Competition", "desc":"The culmination of all your hard work (provisional date)"}]
sr_timeline_start = "September 19, 2009"
sr_timeline_end = "April 17, 2010"

#We want to sanitize <div>s from feeds
feedparser._HTMLSanitizer.acceptable_elements.remove("a")

# The maximum number of messages to display
MAX_MESSAGES = 5
# The maixum length of a message to display (in chars)
MAX_MESSAGE_LENGTH = 100
# The maixum length of time between feed updates (in seconds)
MAX_AGE = 60 * 30 #half an hour

def sanitize_body(text):
	text = text.strip()	#remove whitespace
	end = text.find('<div class="blogger-post-footer">')	#find the footer so we can chop it off
	if end < 0:
		end = MAX_MESSAGE_LENGTH
	else:
		end = min(MAX_MESSAGE_LENGTH,end)
	text = text[0:end]	#ensure its not too long and chop off the footer
	return text

class SRMessageFeed():
	def __init__(self):
		self.ParseFeed()

	def ParseFeed(self):
		#blank existing list of messages
		self.msgs = []
		#note what time the feeds were updated
		self.time = time.time()
		#actually parse the feed
		fd = feedparser.parse(sr_message_feed)
		self.msgs = [{"title":e.title, "link":e.link, "author":e.author_detail.name,
					"date":e.date, "body":sanitize_body(e.description)}
				for e in fd.entries]

	def GetMessages(self):
		"""return a dict of reasonably up-to-date messages"""
		if time.time() > (self.time + MAX_AGE):
			self.ParseFeed()
		#print "SR Feed:\n",self.msgs
		return dict(msgs=self.msgs[0:MAX_MESSAGES])

class StudentBlogPosts():
	def __init__(self):
		self.msgs = []
		self.ParseValidatedFeeds()

	def ParseValidatedFeeds(self):
		#blank existing list of messages
		self.msgs = {}
		#note what time the feeds were updated
		self.time = time.time()
		#get all feeds which have been validated
		try:
			allfeeds = model.UserBlogFeeds.selectBy(valid=True)
		except:
			#either no feeds or an sql error
			return

		for feed in allfeeds:
			print feed
			self.ParseFeed(feed)

	def ParseFeed(self, feed):
		"""
		Parse an actual blog feed passed as a SQLObject for that row
		Add the most recent post on the blog to the message stored against the user whose feed it was
		"""
		fd = feedparser.parse(feed.url)
		try:
			#store just the most recent post
			e = fd.entries[0]
			self.msgs[feed.user] = {"title":e.title, "link":e.link, "author":feed.user,
				"date":e.date, "body":sanitize_body(e.description)}
		except IndexError:
			pass

	def sortpsots(self, a, b):
		"""sort the posts putting the most recent at the top"""
		if a['date'] > b['date']:
			return -1
		elif a['date'] < b['date']:
			return 1
		return 0

	def GetBlogPosts(self):
		"""return a dict of reasonably up-to-date blog posts"""
		if time.time() > (self.time + MAX_AGE):
			self.ParseValidatedFeeds()
		print self.msgs
		msgs = sorted(self.msgs.values(), self.sortpsots)
		return dict(msgs=msgs)

# Single instance of the message feeds shared by all users
sfd = SRMessageFeed()
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
		if cur_user == None:
			return dict(feedurl="", valid=0, error=1)

		# grab the sql record, edit the url, commit it
		try:
			r = model.UserBlogFeeds.selectBy(user=cur_user)
			try:
				row = r.getOne()
				if feedurl != row.url:
					row.valid = False	# will need to be re-validated
					row.checked = False
					row.url = feedurl
					row.set()
				else:
					pass	#nothing to update
			except:
				# user doen't have an entry yet, so create one
				if r.count() == 0:
					row = model.UserBlogFeeds(user=cur_user, url=feedurl, valid=False, checked=False)
					row.set()
				else:
					#multiple results, sql table is corrupt
					return dict(feedurl="", valid=0, checked=0, error=1)
		except:
			#error performing sql query
			return dict(feedurl="", valid=0, checked=0, error=1)
		else:
			#success
			return dict(feedurl=row.url, valid=int(row.valid), checked=int(row.checked), error=0)

	@expose("json")
	@srusers.require(srusers.in_team())
	def getblogfeed(self):
		"""Returns the current rss feed url associated with a user. If
		the user has not previously registered a url, it will return a
		blank string.
		"""
		#get the current user
		cur_user = str(srusers.get_curuser())
		if cur_user == None:
			return dict(feedurl="", valid=0, error=1)

		# grab the sql record for the user, if it exists and extract the url
		try:
			r = model.UserBlogFeeds.selectBy(user=cur_user)
			try:
				row = r.getOne()
			except:
				# the record doesn't exist, return blank
				return dict(feedurl="", valid=0, checked=0, error=0)
		except:
			# the record doesn't exist, return blank
			return dict(feedurl="", valid=0, checked=0, error=0)
		# success, return url
		return dict(feedurl=row.url, valid=int(row.valid), checked=int(row.checked), error=0)

	@expose("json")
	@srusers.require(srusers.in_team())
	def getblogposts(self):
		return sbp.GetBlogPosts()
