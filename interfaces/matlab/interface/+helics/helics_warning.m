function v = helics_warning()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 66);
  end
  v = vInitialized;
end
