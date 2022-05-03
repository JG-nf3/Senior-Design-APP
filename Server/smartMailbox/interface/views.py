from django.shortcuts import render
from django.contrib import auth
from storage.models import Mailbox,Update
def mailboxList(request):
    userId = request.user

    mailboxes = Mailbox.objects.filter(owner = userId.id)
    mailboxes_list = []

    for mailbox in mailboxes:
        updates = Update.objects.filter(mailbox = mailbox).order_by('-id')[:5][::-1]
        mailboxes_list.append({'mailbox':mailbox,'updates':updates})
    context = {'mailboxes':mailboxes_list}

    return render(request, 'storage/interface.html', context=context)