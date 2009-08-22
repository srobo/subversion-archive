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

class SRMessageFeed():
	def __init__(self):
		self.fd = feedparser.parse(sr_message_feed)	

	def GetMessages(self):
		msglist = []
		for e in self.fd.entries:
			msglist.append({"title":e.title, "link":e.link, "author":e.author_detail.name,
					"date":e.date, "body":e.description[0:100]})
		return dict(msgs=msglist)

# Single instance of the message feed shared by all users
sfd =  SRMessageFeed()	

class Switchboard(object):
	@expose("json")
	def getmessages(self):
		return sfd.GetMessages()

	@expose("json")
	def timeline(self):
		#TOOD: get this dynamically, perhaps from an xml file? 
		return dict(start=sr_timeline_start, end=sr_timeline_end, events=sr_timeline_events)
