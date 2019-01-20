function v = helics_warning()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812675);
  end
  v = vInitialized;
end
