from email.policy import default
from django.db import models
from django.contrib import auth

# Create your models here.
class Mailbox(models.Model):
    """Data of a mailbox"""

    unlocked = models.BooleanField(help_text="Is box unlocked", default=False)

    keyCode = models.CharField(max_length=4,help_text="Keycode for the box")

    created = models.DateTimeField(auto_now_add=True, help_text="The date and time that box was created")

    mode = models.IntegerField(help_text="Mode Selection", default=0)

    owner = models.ForeignKey(auth.get_user_model(), on_delete=models.CASCADE)

    name = models.CharField(max_length=20, help_text="The name of the box.", default="My Mailbox")

class Update(models.Model):
    """A data packet of what has happened last"""

    distance = models.FloatField(help_text="distance read by the device", default=0)

    opened = models.BooleanField(help_text="was opened")

    moved = models.BooleanField(help_text="was moved")

    updateTime = models.DateTimeField(auto_now_add=True, help_text="The date and time that box was created")

    mailbox = models.ForeignKey(Mailbox, on_delete=models.CASCADE, help_text="The mailbox this update is for")

    image_update = models.ImageField(upload_to="updates/", default=None)

    read = models.BooleanField(help_text="Was message read", default=True)