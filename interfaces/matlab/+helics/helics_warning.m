function v = helics_warning()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 7);
  end
  v = vInitialized;
end
