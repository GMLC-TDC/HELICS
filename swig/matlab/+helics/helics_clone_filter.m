function v = helics_clone_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107618);
  end
  v = vInitialized;
end
