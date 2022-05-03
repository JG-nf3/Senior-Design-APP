from django.shortcuts import render, redirect
from django.contrib.auth import authenticate,login,logout
from django.contrib import messages
from django.contrib.auth.forms import UserCreationForm
from .forms import RegisterUserForm
from .forms import UpdateForm

# Create your views here.
def login_user(request):
    if request.method == "POST":
        username = request.POST['username']
        password = request.POST['password']
        user = authenticate(request, username=username, password=password)
        if user is not None:
            login(request, user)
            return redirect('home')
        else:
            messages.success(request, ("Error with log in"))
            return redirect('login_user')
    
    else: 
        return render(request, 'authenticate/login_user.html')

def register_user(request):
    if request.method == "POST":
        form = RegisterUserForm(request.POST)
        if form.is_valid():
            form.save()
            username = form.cleaned_data['username']
            password = form.cleaned_data['password1']
            user =authenticate(username=username,password=password)
            login(request, user)
            messages.success(request, ("Register Complete"))
            return redirect('home')
    else:
        form = RegisterUserForm()
        
    return render(request, 'authenticate/register_user.html', {'form':form,})

def logout_user(request):
    logout(request)
    return render(request, 'home/index.html')

def interface_login_user(request):
    if request.method == "POST":
        username = request.POST['username']
        password = request.POST['password']
        user = authenticate(request, username=username, password=password)
        if user is not None:
            login(request, user)
            return redirect('interface')
        else:
            messages.success(request, ("Error with log in"))
            return redirect('login_user')
    
    else: 
        return render(request, 'authenticate/login_user.html')


def user_settings(request):
    if request.method == "POST":
        form = UpdateForm(request.POST)
        if form.is_valid():
            form.save()
            username = form.cleaned_data['username']
            password = form.cleaned_data['password1']
            user =authenticate(username=username,password=password)
            login(request, user)
            messages.success(request, ("Register Complete"))
            return redirect('home')
    else:
        form = UpdateForm()

    return render(request, 'authenticate/user_settings.html', {'form':form,})