from abc import update_abstractmethods
from django.shortcuts import get_object_or_404, render, redirect
from django.contrib import messages
from django.contrib import auth
from .forms import boxForm
from .models import Mailbox

def interface_user(request):
    return render(request, "storage/interface.html")

#PK is primary key
#This method can be used to pull up a form to update an existing box
def mailbox_edit(request, pk=None):
    user = request.user

    mailbox = get_object_or_404(Mailbox, pk=pk)

    if request.method == "POST":
        form = boxForm(request.POST, instance=mailbox)
        if form.is_valid(): 
            updated_box = form.save(False)
            messages.success(request, "Mailbox: \"{}\" was edited".format(updated_box.name))

            updated_box.save()
            return redirect("interface")

    else:
        form = boxForm(instance=mailbox)

    return render(request, "storage/boxEditForm.html", {"method": request.method, "form":form, "model_type": "Mailbox"})

def mailbox_create(request):
    user = request.user

    mailbox = Mailbox(owner=request.user)
    #mailbox = None

    if request.method == "POST":
        form = boxForm(request.POST, instance=mailbox)
        if form.is_valid(): 
            updated_box = form.save(False)
            messages.success(request, "Mailbox: \"{}\" was created".format(updated_box.name))

            updated_box.save()
            return redirect("interface")

    else:
        form = boxForm(instance=mailbox)

    return render(request, "storage/boxForm.html", {"method": request.method, "form":form, "model_type": "Mailbox"})
