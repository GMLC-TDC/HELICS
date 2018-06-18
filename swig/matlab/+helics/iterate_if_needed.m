function v = iterate_if_needed()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176315);
  end
  v = vInitialized;
end
