function v = helics_clone_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176334);
  end
  v = vInitialized;
end
