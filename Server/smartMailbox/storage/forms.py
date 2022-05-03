from dataclasses import fields
from .models import Mailbox
from django import forms

class boxForm(forms.ModelForm):
    class Meta:
        model = Mailbox
        fields = ['name', 'keyCode', 'unlocked']