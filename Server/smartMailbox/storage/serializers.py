from dataclasses import field, fields
from turtle import update
from rest_framework import serializers
from .models import Mailbox,Update

class MailboxSerializer(serializers.ModelSerializer):
    class Meta:
        model = Mailbox
        fields = '__all__'

class UpdateSerializer(serializers.ModelSerializer):
    class Meta:
        model = Update
        fields = '__all__'